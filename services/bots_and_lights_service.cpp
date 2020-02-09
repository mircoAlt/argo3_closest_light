//
// Created by Mirco Altenbernd on 21.11.19.
//

#include "bots_and_lights_service.h"

Light BotsAndLightsService::calculateRelevantLight(const CCI_LootBotLightSensor::TReadings &readings,
                                                   const CCI_PositioningSensor::SReading &position,
                                                   CVector2 *lastLightPosition, string bId,
                                                   BotsAndLightsService *serviceForBot, unsigned long ts) {
    this->botId = bId;
    vector<Light> lights = calculateLights(readings, position);
    vector<BotAt> bots = serviceForBot->getBots(std::move(bId));
    if (lights.empty()) {
        RLOGERR << "WARNING: NO LIGHTS" << endl;
        return {CVector3(0,0,0), -1};
    }
    if (lastLightPosition != nullptr) {
        for (auto &l: lights) {
            if ((l.get2DPosition() - *lastLightPosition).Length() < 0.8) {
                return l;
            }
        }
    }
    auto distancesAndClosestIndex = distancesAndClosestLightForBot(lights, position.Position);
    setClosestLightsForBots(lights, bots, ts);
    Light &closestLight = lights[distancesAndClosestIndex.second];
    if (serviceForBot->useClosestStation()) {
        return closestLight;
    }
    pair<Light&, double> lightForBot = {closestLight, distancesAndClosestIndex.first[distancesAndClosestIndex.second]};
//    BOTLOG << "knows " << lights.size() << " lights, " << closestLight << " is closest, at distance " << lightForBot.second << endl;
    for (int i = 0; i < lights.size(); i++) {
        Light &light = lights[i];
        if (closestLight.getId() == light.getId()) continue;
        double distancesToFurther = distancesAndClosestIndex.first[i];
        double distThreshold = calculateDiffThreshold(distancesToFurther, closestLight, light, bots, ts);
//        BOTLOG << "distThreshold" << distThreshold << endl;
        if (distThreshold >= 0) {
            if (distThreshold < lightForBot.second) {
                lightForBot = {light, distThreshold};

                RLOG << "Bot at ("
                     << position.Position
                     << ") goes to " << lightForBot.first.getId() << " although it is further away" << endl;

            }
        }
    }

    BOTLOG << "going to light at " << lightForBot.first.getPosition() << endl;
    return lightForBot.first;
}

pair<vector<double>, int> BotsAndLightsService::distancesAndClosestLightForBot(const vector<Light>& lights, const CVector2 pos) {
    return distancesAndClosestLightForBot(lights, CVector3(pos.GetX(), pos.GetY(), 0));
}

CRadians
BotsAndLightsService::convertGlobalAngle(const CCI_PositioningSensor::SReading &positionOfBot, const CRadians &angle,
                                            const bool &toGlobal) {
    CRadians zAngle, yAngle, xAngle;
    positionOfBot.Orientation.ToEulerAngles(zAngle, yAngle, xAngle);
//    BOTLOG << "euler angle " << zAngle << endl;
    CRadians normalizedAngle = (angle + (toGlobal? zAngle : - zAngle)).SignedNormalize();
    return normalizedAngle;
}

///// HELPER
bool BotsAndLightsService::printRanges(const vector<vector<int>> &indices) const {
    if (indices.empty() || indices[0].empty()) {
        RLOGERR << "indices empty: " << indices.empty() << ", [0] empty: " << indices[0].empty() << endl;
        return false;
    }
    BOTLOG << "ranges: " << endl;
    for (auto &range : indices) {
        string result = "[";
        for (auto &i : range) {
            result += " " + to_string(i) + " ";
        }
        BOTLOG << result << "]" << endl;
    }
    return true;
//    }
}

pair<CRadians, double>
BotsAndLightsService::getClosestFreeAngle(CVector2 vecToGoal, const CCI_PositioningSensor::SReading &positionOfBot,
                                          long timestamp, bool aimingAtLight, const string& bId, BotsAndLightsService *serviceForBot) {
    botId = bId;
    auto globalAngle = convertGlobalAngle(positionOfBot, vecToGoal.Angle(), true);
    auto currentOr = convertGlobalAngle(positionOfBot, CRadians(), true);
    double importanceOfDirection = 1 - (0.2 * min(2.0, vecToGoal.Length()));
    if (!aimingAtLight) importanceOfDirection *= 0.35;
    if (isnan(globalAngle.GetValue()) || isnan(currentOr.GetValue())) {
        RLOGERR << "invalid call with params" << "vecToGoal " << vecToGoal << ", positionOfBot "
        << positionOfBot.Position << ", timestamp " << timestamp << ", aimingAtLight" << aimingAtLight << endl;
        RLOGERR << "=> globalAngle" << globalAngle << ", currentOr" << currentOr << ", importanceOfDirection" << importanceOfDirection << endl;
        return {CRadians::ZERO, 0.0};
    }
    ScoreWheel wheel(globalAngle, currentOr, GetId(), importanceOfDirection);
    vector<BotAt> currentBots;
    auto bots = serviceForBot->getBots(bId);
    if (!bots.empty()) copy_if (bots.begin(), bots.end(), back_inserter(currentBots),
            [this, timestamp, bId](BotAt &bot){
        bool second = timestamp - bot.timestamp < 100;
        bool third = bot.GetId() != bId;
//        BOTLOG << "first: " << first << ", second " << second << ", third: " << third << endl;
        return second && third;
    });
//    BOTLOG << "verifying " << currentBots.size() << " bots" << endl;
    wheel.updateScores(currentBots, walls, CVector2(positionOfBot.Position.GetX(), positionOfBot.Position.GetY()), timestamp);
    auto winner = wheel.getWinningAngle();
    auto winnerAngle = winner.getClosestFor(currentOr);
//    if (Abs(NormalizedDifference(winnerAngle, currentOr)) >= ToRadians(CDegrees(10.05))){
//        BOTLOG << "winner: " << winner << ", angle " << winnerAngle << ", original: " << globalAngle << ", current: " << currentOr << endl;
//    }
    return {convertGlobalAngle(positionOfBot, winnerAngle), winner.score.speed};
}

double BotsAndLightsService::calculateDiffThreshold(double distanceToFurther, Light closestLight, Light furtherLight,
                                                    vector<BotAt> &bots, unsigned long ts) {
    int atThisLight = 1;
    int atOtherLight = 0;
    vector<double> distances = { distanceToFurther };
    for (auto &bot : bots) {
        if (bot.closestLight->getId() == closestLight.getId()) {
            atThisLight++;
            distances.push_back((bot.assumedPosition(ts) - furtherLight.get2DPosition()).Length());
        } else if (bot.closestLight->getId() == furtherLight.getId()) {
            atOtherLight++;
        }
    }
//    BOTLOG << "distanceToFurther: " << distanceToFurther << " atThisLight: " << atThisLight
//                             << " atOtherLight: " << atOtherLight << " distances: " << distances.size() << endl;
    return closestLight.calculateDiffThreshold(distanceToFurther, atThisLight, atOtherLight, distances);

}

void BotsAndLightsService::setClosestLightsForBots(vector<Light> &lights, vector<BotAt> &bots, unsigned long ts) {
    for (auto &bot: bots) {
        auto dists = distancesAndClosestLightForBot(lights, bot.assumedPosition(ts));
//        if (bot.bot != nullptr) BOTLOG << "closest light for " << bot.bot->entity.GetId() << " is light " << lights[dists.second].getId() << endl;
        bot.closestLight = &(lights[dists.second]);
    }

}

pair<vector<double>, int> BotsAndLightsService::distancesAndClosestLightForBot(const vector<Light>& lights, const CVector3 pos) {
    double minDist = 1000;
    int minIndex = 0;
    vector<double> distances;
    for (int i = 0; i < lights.size(); i++) {
        double dist = (lights[i].getPosition() - pos).Length();
        distances.push_back(dist);
        if (dist < minDist) {
            minDist = dist;
            minIndex = i;
        }
    }
    return {distances, minIndex};
}

bool BotsAndLightsService::isWall(CVector2 position) {
    WallAt wallOnX(position, true);
    WallAt wallOnY(position, false);
    auto foundWallOnX = find(walls.begin(), walls.end(), wallOnX);
    auto foundWallOnY = find(walls.begin(), walls.end(), wallOnY);
    return foundWallOnX != walls.end() || foundWallOnY != walls.end();
}

bool BotsAndLightsService::isAimingAtWall(const CCI_PositioningSensor::SReading &positionOfBot, CRadians aiming) {
    CVector2 inFront =  CVector2(0.5, aiming) + CVector2(positionOfBot.Position.GetX(), positionOfBot.Position.GetY());
    return isWall(inFront);
}

void BotsAndLightsService::insertWall(const string &bId, const CVector2 &vecToWall, const bool &isOnX) {
    WallAt wall(vecToWall, isOnX);
    walls.insert(wall);
}

int BotsAndLightsService::noOfBotsNearby(CVector2 positionOfBot,
                                         string forBotId, unsigned long ts) {
    int counter = 0;
    for (auto &bot: getBots(forBotId)) {
        if ((bot.assumedPosition(ts)-positionOfBot).Length() < ScoreWheel::distThreshold) {
            counter++;
        }
    }
    return counter;
}
