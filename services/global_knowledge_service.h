//
// Created by Mirco Altenbernd on 21.11.19.
//

#ifndef ARGOS3_EXAMPLES_GLOBAL_KNOWLEDGE_SERVICE_H
#define ARGOS3_EXAMPLES_GLOBAL_KNOWLEDGE_SERVICE_H

#include "bots_and_lights_service.h"
#include <models/bot.h>
class GlobalKnowledgeService: public BotsAndLightsService {
public:
    vector<Light>
    calculateLights(const CCI_LootBotLightSensor::TReadings &readings, const CCI_PositioningSensor::SReading &position) override;
    void addBots(CCI_LootBotProximitySensor::TReadings readings, const CCI_PositioningSensor::SReading &position,
                 long timestamp,
                 string botId, BotsAndLightsService *serviceForWalls) override;
    void addWalls(CCI_LootBotProximitySensor::TReadings readings, const CCI_PositioningSensor::SReading &position,
                  long timestamp, string botId) override;

    GlobalKnowledgeService(vector<Light> lights, const vector<Bot*>& actualBots, set<WallAt> walls);
    pair<bool, bool> isNewWall(const CCI_LootBotProximitySensor::TReadings &readings,
                               const CCI_PositioningSensor::SReading &positionOfBot,
                               vector<int> &obs) override;

    void update();
    vector<BotAt> getBots(string ownId) override;

    void disableCriteria();

    bool useClosestStation() override;

private:
    vector<Light> lights;
    vector<Bot*> actualBots;
    bool closestStation = false;


};


#endif //ARGOS3_EXAMPLES_GLOBAL_KNOWLEDGE_SERVICE_H
