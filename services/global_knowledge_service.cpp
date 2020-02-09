//
// Created by Mirco Altenbernd on 21.11.19.
//

#include "global_knowledge_service.h"

#include <utility>

vector<Light> GlobalKnowledgeService::calculateLights(const CCI_LootBotLightSensor::TReadings &readings,
                                                      const CCI_PositioningSensor::SReading &position) {
    return lights;
}

GlobalKnowledgeService::GlobalKnowledgeService(vector<Light> lights, const vector<Bot*>& actualBots, set<WallAt> walls)
    : lights(std::move(lights)) {
    RLOG << "created gk service with " << walls.size() << " walls and " << actualBots.size() << " bots" << endl;
    this->walls = std::move(walls);
    for (auto *b: actualBots) {
        this->actualBots.emplace_back(b);
    }
}

void GlobalKnowledgeService::update() {
    for (auto *b: actualBots) {
        b->update();
    }
}

void GlobalKnowledgeService::addBots(CCI_LootBotProximitySensor::TReadings readings,
                                     const CCI_PositioningSensor::SReading &position, long timestamp,
                                     string botId, BotsAndLightsService *serviceForWalls) {

}

vector<BotAt> GlobalKnowledgeService::getBots(string ownId) {
    vector<BotAt> bots;
    for (auto *b: actualBots) {
        if (b->GetId() != ownId) {
            bots.emplace_back(*b);
        }
    }
    return bots;
}

pair<bool, bool>
GlobalKnowledgeService::isNewWall(const CCI_LootBotProximitySensor::TReadings &readings,
                                  const CCI_PositioningSensor::SReading &positionOfBot,
                                  vector<int> &obs) {
    return { false, false};
}

void GlobalKnowledgeService::disableCriteria() {
    closestStation = true;
}

bool GlobalKnowledgeService::useClosestStation() {
    return closestStation;
}

void GlobalKnowledgeService::addWalls(CCI_LootBotProximitySensor::TReadings readings,
                                      const CCI_PositioningSensor::SReading &position, long timestamp, string botId) {}
