//
// Created by Mirco Altenbernd on 21.11.19.
//

#ifndef ARGOS3_EXAMPLES_BOTS_AND_LIGHTS_SERVICE_H
#define ARGOS3_EXAMPLES_BOTS_AND_LIGHTS_SERVICE_H


#include <argos3/plugins/robots/loot-bot/control_interface/ci_lootbot_light_sensor.h>
#include <argos3/plugins/robots/loot-bot/control_interface/ci_lootbot_proximity_sensor.h>
#include <argos3/core/utility/math/vector2.h>
#include <argos3/plugins/robots/generic/control_interface/ci_positioning_sensor.h>
#include <models/light.h>
#include <models/bot_at.h>
#include <models/wall_at.h>
#include <services/score_wheel.h>
#include "score_wheel.h"
#include "debug_log.h"
#include <utility>
#include <models/bot_at.h>
#include <argos3/core/utility/logging/argos_log.h>
#include <argos3/core/utility/math/rng.h>

#include <utility>

class BotsAndLightsService {

    static CRadians convertGlobalAngle(const CCI_PositioningSensor::SReading &positionOfBot, const CRadians &angle,
                                const bool &toGlobal = false);
    static double calculateDiffThreshold(double distanceToFurther, Light closestLight, Light furtherLight,
                                         vector<BotAt> &bots, unsigned long ts);

protected:
    struct ObstaclePositions {
        CVector2 asBot;
        CVector2 asWall;
        vector<int> readingIndices;
        ObstaclePositions(CVector2 asBot, CVector2 asWall, vector<int> readingIndices) : asBot(asBot), asWall(asWall), readingIndices(std::move(readingIndices)) {}
    };

    static pair<vector<double>, int> distancesAndClosestLightForBot(const vector<Light> &lights, CVector2 pos);

    bool printRanges(const vector<vector<int>> &indices) const;

    set<WallAt> walls;

    virtual vector<BotAt> getBots(string ownId) = 0;
    virtual vector<Light>
    calculateLights(const CCI_LootBotLightSensor::TReadings &readings, const CCI_PositioningSensor::SReading &position) = 0;

    virtual bool useClosestStation() {
        return false;
    }

public:
    inline string GetId() const {
        return botId;
    }

    bool doDebug = false;
    string botId;

    Light calculateRelevantLight(const CCI_LootBotLightSensor::TReadings &readings,
                                 const CCI_PositioningSensor::SReading &position,
                                 CVector2 *lastLightPosition, string bId,
                                 BotsAndLightsService *serviceForBot, unsigned long ts);

    pair<CRadians, double> getClosestFreeAngle(CVector2 vecToGoal, const CCI_PositioningSensor::SReading &positionOfBot,
                                               long timestamp, bool aimingAtLight, const string& bId, BotsAndLightsService *serviceForBot);

    virtual void
    addBots(CCI_LootBotProximitySensor::TReadings readings, const CCI_PositioningSensor::SReading &position,
            long timestamp,
            string botId, BotsAndLightsService *serviceForWalls) = 0;

    virtual void
    addWalls(CCI_LootBotProximitySensor::TReadings readings, const CCI_PositioningSensor::SReading &position,
             long timestamp, string botId) = 0;

    virtual pair<bool, bool> isNewWall(const CCI_LootBotProximitySensor::TReadings &readings,
                                       const CCI_PositioningSensor::SReading &positionOfBot,
                                       vector<int> &obs) = 0;

    static void setClosestLightsForBots(vector<Light> &lights, vector<BotAt> &bots, unsigned long ts);

    static pair<vector<double>, int> distancesAndClosestLightForBot(const vector<Light> &lights, CVector3 pos);

    bool isWall(CVector2 position);

    bool isAimingAtWall(const CCI_PositioningSensor::SReading &positionOfBot, CRadians aiming);

    void insertWall(const string &botId, const CVector2 &vecToWall, const bool &isOnX);

    int noOfBotsNearby(CVector2 positionOfBot, string forBotId, unsigned long ts);
};


#endif //ARGOS3_EXAMPLES_BOTS_AND_LIGHTS_SERVICE_H
