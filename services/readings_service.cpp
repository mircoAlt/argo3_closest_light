
#include <argos3/core/utility/math/rng.h>
#include <argos3/core/utility/logging/argos_log.h>

#include <utility>
#include "readings_service.h"
#include "debug_log.h"
#include "bots_and_lights_service.h"

///// COMMON
vector<vector<int>>
ReadingsService::getIndicesForSameReading(const vector<Real> &readings, bool replaceSingleZeros) const {
    vector<vector<int>> sameReadings= { { } };
    int minsCounter = 0;
    for(int i = 0; i < readings.size(); i++) {
        unsigned long previousIndex = (i < 1) ? readings.size() - 1 : i - 1;
        int nextIndex = (i < readings.size() - 1) ? i + 1 : 0;
        if (readings[previousIndex] >= readings[i]) {
            if (readings[nextIndex] > readings[i]) {
                if (!replaceSingleZeros || readings[i] != 0 || readings[previousIndex] <= 0.4 ||
                    readings[nextIndex] <= 0.4) {
                    if (!sameReadings[minsCounter].empty()) {
                        minsCounter++;
                        sameReadings.emplace_back();
                    }
                    continue;
                }
            } else if (readings[nextIndex] == readings[i]) {
                continue;
            }
        }
        sameReadings[minsCounter].push_back(i);

    }
    int last = sameReadings.size() - 1;
    // merge last and first entry since readings are circular
    if (last > 0) {
        int lastLast = sameReadings[last].size() - 1;
        if (lastLast == - 1) {
            sameReadings.pop_back();
        } else if (lastLast >= 0 && sameReadings[0][0] == 0 && sameReadings[last][lastLast] == readings.size() - 1) {
            auto end = sameReadings[last];
            sameReadings[0].insert(sameReadings[0].end(), end.begin(), end.end());
            sameReadings.pop_back();
        }
    }
    return sameReadings;
}

///// LIGHT

vector<Light> ReadingsService::calculateLights(const CCI_LootBotLightSensor::TReadings &readings,
        const CCI_PositioningSensor::SReading &position) {
    vector<Real> values;
    for (auto &reading: readings) {
        values.push_back(reading.Value);
    }
    vector<vector<int>> readingsForSameLight = getIndicesForSameReading(values);
    if(!printRanges(readingsForSameLight)) {
        return {  };
    }
    vector<Light> result;
    for (auto &i: readingsForSameLight) {
        Real lengthResult = 0;
        CRadians mainAngle = getMainAngleFor(i, readings);
        int counter = 0;
        for ( auto &r : i) {
            Real length = calculateLightDistance(readings[r].Value, readings[r].Angle, mainAngle);
            if (length > 0) {
                lengthResult += length;
                counter ++;
            }
            BOTLOG << "length: " << length << endl;
            BOTLOG << "counter: " << counter << endl;

        }
        if (counter <= 0) continue;
        lengthResult /= counter;
        CVector2 vec = toGlobalPosition(position, mainAngle, lengthResult);
        int id = (int) Abs((vec.GetX() + vec.GetY()) * 1000.0);
        Light light(CVector3(vec.GetX(), vec.GetY(), 0), id);
        BOTLOG << "found light " << light << endl;
        result.push_back(light);
    }

    return result;
}

//// BOTS
void ReadingsService::addBots(CCI_LootBotProximitySensor::TReadings readings,
                              const CCI_PositioningSensor::SReading &position, long timestamp,
                              string botId, BotsAndLightsService *serviceForWalls) {
    this->botId = botId;
    bool forBot = true;
    auto positions = calculateObstaclePosition(readings, position);
    for (auto &pos: positions) {
            if(serviceForWalls->isWall(pos.asWall)) {
//                RLOG << "is wall at " << vecToWall << endl;
                updateBot(pos.asBot, timestamp, false, 0);
            } else {
                updateBot(pos.asBot, timestamp, true, pos.readingIndices.size());
            }
    }
}

void ReadingsService::addWalls(CCI_LootBotProximitySensor::TReadings readings,
                              const CCI_PositioningSensor::SReading &position, long timestamp,
                              string botId) {
    this->botId = botId;
    auto positions = calculateObstaclePosition(readings, position);
    for (auto &pos: positions) {
        if (!isWall(pos.asWall)) {
            auto newWallInfo = isNewWall(readings, position, pos.readingIndices);
            if (newWallInfo.first) {
                RLOG << "detected wall at " << pos.asWall << endl;
                insertWall(botId, pos.asWall, newWallInfo.second);
            }
        }
    }
}

vector<BotsAndLightsService::ObstaclePositions> ReadingsService::calculateObstaclePosition(const CCI_LootBotProximitySensor::TReadings &readings,
                                                            const CCI_PositioningSensor::SReading &position) const {
    vector<vector<int>> sameObs = getIndicesForSameReading(readings);
    CCI_LootBotProximitySensor::SReading read;
    vector<BotsAndLightsService::ObstaclePositions> results;
    for (auto &obs: sameObs) {

        CVector2 vec;
        for (auto &i : obs) {
            double notNullValue = (readings[i].Value == 0) ? 1 : readings[i].Value;
            vec += CVector2(notNullValue, readings[i].Angle);
        }
        vec /= obs.size();
        read = CCI_LootBotProximitySensor::SReading(vec.Length(), vec.Angle());
        if (read.Value > 0) {
            double distToObstacle = calculateBotDistance(read);
            // add additional radius to get center of bot
            CVector2 vecToBot = toGlobalPosition(position, read.Angle,distToObstacle + BotAt::radiusOfRobot);
            CVector2 vecToWall = toGlobalPosition(position, read.Angle,distToObstacle );
            results.emplace_back(vecToBot, vecToWall, obs);
        }
    }
    return results;
}

CVector2 ReadingsService::toGlobalPosition(const CCI_PositioningSensor::SReading &positionOfBot,
        const CRadians &angle, const double &distance) const {
    CVector2 posOn2D;
    positionOfBot.Position.ProjectOntoXY(posOn2D);
    CRadians xAngle, yAngle, zAngle;
    positionOfBot.Orientation.ToEulerAngles(xAngle, yAngle, zAngle);
    CRadians normalizedAngle = angle + xAngle;
    CVector2 vecToBot = CVector2(distance, normalizedAngle);
    vecToBot += posOn2D;
    return vecToBot;
}

pair<bool, bool> ReadingsService::isNewWall(const CCI_LootBotProximitySensor::TReadings &readings,
                                            const CCI_PositioningSensor::SReading &positionOfBot,
                                            vector<int> &obs) {
    if (obs.size() >= 3) {
        int zero = obs[0];
        int one = obs[1];
        int two = obs[2];
        double leftLength = calculateBotDistance(readings[zero]);
        if (leftLength < 1.4 * BotAt::radiusOfRobot) return {false, false};
        double middleLength = calculateBotDistance(readings[one]);
        if (middleLength < 1.4 * BotAt::radiusOfRobot) return {false, false};
        double rightLength = calculateBotDistance(readings[two]);
        if (rightLength < 1.4 * BotAt::radiusOfRobot) return {false, false};

        CVector2 leftSide = toGlobalPosition(positionOfBot, readings[zero].Angle, leftLength);
        CVector2 middleSide = toGlobalPosition(positionOfBot, readings[one].Angle, middleLength);
        CVector2 veryRightSide = toGlobalPosition(positionOfBot, readings[two].Angle, rightLength);
        CVector2 connector = middleSide - leftSide;
        CRadians secondAngle = (middleSide - veryRightSide).Angle();
        auto angle = ((connector.Angle() - secondAngle) - CRadians::PI).SignedNormalize();
        bool isWall = Abs(angle) < ToRadians(CDegrees(1.5));
        bool onX = isWall ? Abs(connector.GetY()) < Abs(connector.GetX()) : false;
        return {isWall, onX};
    }
    return {false, false};
}

vector<vector<int>> ReadingsService::getIndicesForSameReading(const CCI_LootBotProximitySensor::TReadings& readings) const {
    vector<Real> values;
    for (auto &reading: readings) {
        values.push_back(reading.Value);
    }
    return getIndicesForSameReading(values, true);
}

void ReadingsService::updateBot(CVector2 position, long timestamp, bool insert, int noOfReadings) {
    auto botAt = BotAt(position, noOfReadings, timestamp);
    auto existingBot = find(bots.begin(), bots.end(), botAt);
    if (existingBot == bots.end()) {
        if(insert) {
            BOTLOG << "adding bot at " << position << " at time " << timestamp << endl;
            bots.push_back(botAt);
        }
    } else {
        if (!insert) {
            bots.erase(existingBot);
            BOTLOG << "removing bot at " << position << endl;
        } else {
            existingBot->update(botAt);
        }
    }

}

CRadians ReadingsService::getMainAngleFor(vector<int> &range, const CCI_LootBotLightSensor::TReadings &readings) {
    CVector2 vec;
    for (auto &r: range) {
        vec += CVector2(readings[r].Value, readings[r].Angle);
    }
    return vec.Angle();
}

vector<BotAt> ReadingsService::getBots(string ownId) {
    return bots;
}

ReadingsService::ReadingsService() : bots({}) {
    walls = {};
}

double ReadingsService::calculateLightDistance(double intensity, CRadians angleForReading, CRadians mediumReading) {
    CRadians diff = Abs((angleForReading - mediumReading).SignedNormalize());
    if (diff > CRadians::PI_OVER_TWO) {
        return 0.00001f;
    }
    double scale = (1.0f - 2.0f *  diff / CRadians::PI);
    return actualIntensity * log(intensity / scale) / -2.0;
}

double ReadingsService::calculateBotDistance(CCI_LootBotProximitySensor::SReading read) const {
    return 0.0100527 / read.Value - 0.000163144 + BotAt::radiusOfRobot;
}