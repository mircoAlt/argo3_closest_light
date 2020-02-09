//
// Created by Mirco Altenbernd on 30.10.19.
//

#ifndef ARGOS3_EXAMPLES_READINGSSERVICE_H
#define ARGOS3_EXAMPLES_READINGSSERVICE_H
#include <argos3/plugins/robots/loot-bot/control_interface/ci_lootbot_light_sensor.h>
#include <argos3/plugins/robots/loot-bot/control_interface/ci_lootbot_proximity_sensor.h>
#include <argos3/core/utility/math/vector2.h>
#include <argos3/plugins/robots/generic/control_interface/ci_positioning_sensor.h>
#include <models/light.h>
#include <models/bot_at.h>
#include <models/wall_at.h>
#include <services/score_wheel.h>
#include "bots_and_lights_service.h"
#include "score_wheel.h"
#include <models/bot_at.h>

using namespace argos;
using namespace std;
class ReadingsService : public BotsAndLightsService {
public:

    void addBots(CCI_LootBotProximitySensor::TReadings readings, const CCI_PositioningSensor::SReading &position,
                 long timestamp,
                 string botId, BotsAndLightsService *serviceForWalls) override;

    void addWalls(CCI_LootBotProximitySensor::TReadings readings, const CCI_PositioningSensor::SReading &position,
                  long timestamp, string botId) override;
    void updateBot(CVector2 position, long timestamp, bool insert, int noOfReadings);

//    void addBotInstances(const vector<Bot *> &botInstances, long timestamp);

    vector<Light>
    calculateLights(const CCI_LootBotLightSensor::TReadings &readings, const CCI_PositioningSensor::SReading &position) override;

    vector<BotAt> getBots(string ownId) override;
    pair<bool, bool> isNewWall(const CCI_LootBotProximitySensor::TReadings &readings,
                               const CCI_PositioningSensor::SReading &positionOfBot,
                               vector<int> &obs) override;

    ReadingsService();

    static double calculateLightDistance(double intensity, CRadians angleForReading, CRadians mediumReading);

private:
    static const int actualIntensity = 7;

    vector<BotAt> bots;

    vector<vector<int>> getIndicesForSameReading(const vector<Real> &readings, bool replaceSingleZeros = false) const;

    vector<vector<int>> getIndicesForSameReading(const CCI_LootBotProximitySensor::TReadings &readings) const;

    CVector2 toGlobalPosition(const CCI_PositioningSensor::SReading &positionOfBot, const CRadians &angle,
                              const double &distance) const;

    CRadians getMainAngleFor(vector<int> &range, const CCI_LootBotLightSensor::TReadings &readings);

protected:
    double calculateBotDistance(CCI_LootBotProximitySensor::SReading read) const;

    vector<BotsAndLightsService::ObstaclePositions> calculateObstaclePosition(const CCI_LootBotProximitySensor::TReadings &readings,
                                               const CCI_PositioningSensor::SReading &position) const;
};

#endif //ARGOS3_EXAMPLES_READINGS_SERVICE_H
