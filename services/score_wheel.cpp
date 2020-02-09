#include "score_wheel.h"

#include <utility>
#include "debug_log.h"

ScoreWheel::ScoreWheel(CRadians direction, CRadians currentDirection, string id, double importanceOfDirection) : id(std::move(id)) {
    CRadians areaSize = CRadians::TWO_PI / NO_OF_AREAS;
    for (int i = 0; i < NO_OF_AREAS; i++) {
        areas.emplace_back((areaSize * i) + direction, direction, currentDirection, importanceOfDirection);
    }
}

void ScoreWheel::updateScores(const vector<BotAt> &bots, const set<WallAt> &walls, const CVector2 &botPosition,
                              unsigned long ts) {
    CRadians areaSize = CRadians::TWO_PI / NO_OF_AREAS;
    bool doDebug = false;
    if (!bots.empty()) {
        for (auto &a: areas) {
            BOTLOG << a << endl;
        }
        BOTLOG << "own position " << botPosition << endl;
    }
    for (auto &bot: bots) {
        CVector2 vecToBot = bot.assumedPosition(ts, true) - botPosition;
        Real length = vecToBot.Length();
        CRadians angle = vecToBot.Angle().SignedNormalize();
//        Real length = ((int) (vecToBot.Length() * 100)) / 100.0;
//        CRadians actAngle = vecToBot.Angle().SignedNormalize();
//        CRadians angle = ToRadians(CDegrees((int) ToDegrees(actAngle).GetValue()));
//        BOTLOG << "bot at angle " << angle << " has length " << length << endl;
        if (length >= distThreshold) {
            continue;
        }
        if (bot.movesPerTs != CVector2::ZERO) {
            CRadians directionOfBotComparedToMe = Abs(NormalizedDifference(angle, bot.movesPerTs.Angle()));
//            cout << "Norm (" << angle << ", " << bot.movesPerTs.Angle() << ") = " << directionOfBotComparedToMe << endl;
            if (directionOfBotComparedToMe <= CRadians::PI_OVER_TWO) {
                updateSpeeds(directionOfBotComparedToMe, getWheelIndexFor(angle));
                return;
            }
        }
        updateRelevances(distThreshold -length, angle);
    }

    for (auto &w: walls) {
        double length = w - botPosition;
//        BOTLOG << "wall " << w <<  " has length " << length << endl;
        if (length < wallDistThreshold) {
            CRadians angle = w.angleFrom(botPosition);
//            BOTLOG << "wall at angle " << angle << " with distance " << length << endl;
//            BOTLOG << "current max " << getWinningAngle() << endl;
            updateRelevances(distThreshold - BotAt::radiusOfRobot - length, angle);
//            BOTLOG << "new max " << getWinningAngle() << endl;
        }
    }
    if (!bots.empty()) {
        for (auto &a: areas) {
            BOTLOG << a << endl;
        }
    }
}

void ScoreWheel::updateRelevances(const Real &length, const CRadians &angle) {
    int angleSplit = 151; // use an uneven number to not hit the boundaries
    CRadians currentAngle;
    for (int j = 0; j < angleSplit; j++) {
        currentAngle = (CRadians::PI / angleSplit) * j;
        double updater = 2 * length / (j + 1);
        for (auto &updateAngle : {angle + currentAngle, angle - currentAngle}) {
            int i = getWheelIndexFor(updateAngle);
            if (i >= 0) {
                areas[i].score.relevance -= updater;
            }
        }
    }
}

void ScoreWheel::updateSpeeds(const CRadians &angle, const int &wheelIndex) {
    bool doDebug = false;
    BOTLOG << "angle to otherBotDirection: " << angle << endl;
    double updater = (CRadians::PI_OVER_TWO - angle) / CRadians::PI_OVER_TWO;
    BOTLOG << "multipling " << updater << " with " << wheelIndex
        << " => " << areas[wheelIndex].score.speed * updater << endl;
    if (wheelIndex >= 0) {
        areas[wheelIndex].score.speed *= updater;
    }
}

int ScoreWheel::getWheelIndexFor(CRadians angle) {
    auto norm = angle.SignedNormalize();
    for (int i = 0; i < NO_OF_AREAS; i++) {
        if (areas[i].includes(norm)) {
            return i;
        }
    }
    RLOGERR << "could not get area for angle " << angle << endl;
    for (auto &a: areas) {
        RLOGERR << a << endl;
    }
    return -1;
}

ScoreWheel::WheelArea ScoreWheel::getWinningAngle() {
    bool doDebug = false;
    for (auto &a: areas) {
        BOTLOG << a << endl;
    }
    int index = max_element(areas.begin(), areas.end()) - areas.begin();
    return areas[index];
}
