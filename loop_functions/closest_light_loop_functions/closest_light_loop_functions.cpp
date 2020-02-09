#include "closest_light_loop_functions.h"
#include <controllers/lootbot_closest_light/lootbot_closest_light.h>
#include <thread>
#include <services/debug_log.h>

using namespace std;
/****************************************/
/****************************************/

ClosestLightLoopFunctions::ClosestLightLoopFunctions() :
        finishedBots(unordered_set<string>()),
        bots(vector<Bot*>()),
        service(nullptr),
        gkService(nullptr) {}

/****************************************/
/****************************************/

void ClosestLightLoopFunctions::Init(TConfigurationNode &node) {
    auto lightParams = readLightParams(node);
    glConfig = readGlParams(node);
    service = new LightService(lightParams);
    createLights();
    DLOG << "init" << endl;
    initBots();
    service->printLights();
    if (glConfig.needsGlobalKnowledge()) {
        vector<Light> lights;
        for (auto *l: service->getAllLights()) {
            lights.push_back(*l);
        }
        WallAt wallNorth({0, 9.9}, true);
        WallAt wallSouth({0, -9.9}, true);
        WallAt wallEast({4.9, 0}, false);
        WallAt wallWest({-4.9, 0}, false);
        gkService = new GlobalKnowledgeService(
                lights, bots, {wallNorth, wallSouth, wallEast, wallWest});
        gkService->doDebug = doDebug;
    }
    for (Bot *bot : bots) {
        BotsAndLightsService* readingService = (glConfig.needsReadingService()) ? new ReadingsService() : nullptr;
        if (readingService != nullptr) {
            RLOG << "created reading service for bot " << bot->entity->GetId() << endl;
            readingService->doDebug = doDebug;
        } else {
            RLOG << "created no reading service for bot " << bot->entity->GetId() << endl;
        }
        auto &controller = dynamic_cast<CLootBotClosest &>(bot->entity->GetControllableEntity().GetController());
        controller.balServices.forLights =  (glConfig.lights) ? gkService : readingService;
        controller.balServices.fotWalls = (glConfig.walls) ? gkService : readingService;
        bool globalFc = false;
        switch (glConfig.botsFc) {
            case BOTS_LOCAL:
                controller.balServices.fotBotsFc = readingService;
                break;
            case CLOSEST:
                gkService->disableCriteria();
            case BOTS_GLOBAL:
                controller.balServices.fotBotsFc = gkService;
                globalFc = true;
                break;
        }
        if (globalFc != glConfig.botsOa) {
            controller.balServices.fotBotsOa = (glConfig.botsOa) ? gkService : readingService;
        }
    }

}

LightService::SLightParams ClosestLightLoopFunctions::readLightParams(TConfigurationNode &node) {
    try {
        auto lightParams = LightService::SLightParams();
        auto lightNode = GetNode(node, "light");
        GetNodeAttribute(lightNode, "dist", lightParams.lightDist);
        GetNodeAttribute(lightNode, "amount", lightParams.noOfLights);

        auto arenaNode = GetNode(node, "arena");
        GetNodeAttribute(arenaNode, "xLength", lightParams.arenaSideXLength);
        GetNodeAttribute(arenaNode, "yLength", lightParams.arenaSideYLength);

        return lightParams;
    }
    catch (CARGoSException &ex) {
        THROW_ARGOSEXCEPTION_NESTED("Error initializing loop function light & arena parameters.", ex);
    }
}

ClosestLightLoopFunctions::SGlobalLocalConfig ClosestLightLoopFunctions::readGlParams(TConfigurationNode &node) {
    try {
        auto glParams = SGlobalLocalConfig();
        auto glNode = GetNode(node, "gl");
        int botsFcBuffer;
        GetNodeAttribute(glNode, "lights", glParams.lights);
        GetNodeAttribute(glNode, "botsFc", botsFcBuffer);
        GetNodeAttribute(glNode, "botsOa", glParams.botsOa);
        GetNodeAttribute(glNode, "walls", glParams.walls);
        glParams.botsFc = static_cast<CriteriaMode >(botsFcBuffer);

        return glParams;
    }
    catch (CARGoSException &ex) {
        THROW_ARGOSEXCEPTION_NESTED("Error initializing loop function light & arena parameters.", ex);
    }
}

void ClosestLightLoopFunctions::Destroy() {
//    service->printDistribution();
}

void ClosestLightLoopFunctions::Reset() {
    initBots();
    service->printLights();
}

void ClosestLightLoopFunctions::PreStep() {
    for (Bot *bot : bots) {
        auto &controller = dynamic_cast<CLootBotClosest &>(bot->entity->GetControllableEntity().GetController());
        if (service->lightParams.noOfLights == -1 && controller.getNextLightPos() != nullptr) {
            MyLightEntity *atLight = service->findLightAt(*controller.getNextLightPos());
            if (atLight != nullptr) {
                CColor color;
                switch (atLight->getId()) {
                    case 0:
                        color = CColor::PURPLE;
                        break;
                    case 1:
                        color = CColor::GREEN;
                        break;
                    default:
                        color = CColor::YELLOW;
                        break;
                }
                bot->entity->GetLEDEquippedEntity().SetAllLEDsColors(color);
            }
        } else {
            double batteryState = controller.getBatteryLevel() / (double) controller.batteryParams.capacity;
            setBotColorFromPercentage(bot, batteryState);
        }
        if (controller.startedCharging) {
            CVector3 botPos = bot->entity->GetEmbodiedEntity().GetOriginAnchor().Position;
            MyLightEntity *atLight = service->findLightAt({botPos.GetX(), botPos.GetY()});
            if (atLight != nullptr) {
                RLOG << "adding bot" << bot->GetId() << " to light " << atLight->GetId() << endl;
                atLight->addToAssignedBots();
            }

        }
//        controller.readingsService->addBotInstances(bots, controller.timestamp);

    }
}


void ClosestLightLoopFunctions::PostStep() {
    if (gkService != nullptr) {
        gkService->update();
    }
}

bool ClosestLightLoopFunctions::IsExperimentFinished() {
    int noOfDeadBots = 0;
    for (auto &bot: bots) {
        auto &controller = dynamic_cast<CLootBotClosest &>(bot->entity->GetControllableEntity().GetController());
        if(controller.dead) {
            noOfDeadBots++;
            if (bot->diedAt == nullptr) {
                bot->diedAt = new int(GetSpace().GetSimulationClock());
            }
        }
    }
    return noOfDeadBots == bots.size();
}

void ClosestLightLoopFunctions::PostExperiment() {
    RLOG << "finished! " << endl;
    auto lightResult = service->printDistribution();
    unsigned int sumDeadTime = 0;
    unsigned int noOfDeadBots = 0;
    for (auto &bot: bots) {
        if (bot->diedAt != nullptr) noOfDeadBots++;
        sumDeadTime += (bot->diedAt != nullptr) ? *(bot->diedAt) : GetSpace().GetSimulationClock();
    }
    double avgDeadTime = (double) sumDeadTime / bots.size();
    RLOG << noOfDeadBots << " died" << endl;
    std::ofstream m_cOutput;
    auto seed = CSimulator::GetInstance().GetRandomSeed();
    string filename = "closest_" + to_string(seed) +".txt";
    m_cOutput.open(filename, std::ios_base::app | std::ios_base::out);
    m_cOutput << glConfig << "; " <<
        GetSpace().GetSimulationClock() << "; " <<
        avgDeadTime << "; " <<
        lightResult << std::endl;
    m_cOutput.close();
}

void ClosestLightLoopFunctions::initBots() {
    CSpace::TMapPerType &lootbots = GetSpace().GetEntitiesByType("loot-bot");
    for (auto &lootbot : lootbots) {
        /* Get handle to loot-bot entity and controller */
        CLootBotEntity &lootBotEntity = *any_cast<CLootBotEntity *>(lootbot.second);
        auto &controller = dynamic_cast<CLootBotClosest &>(lootBotEntity.GetControllableEntity().GetController());
        controller.doDebug = doDebug;
        CVector3 position = lootBotEntity.GetEmbodiedEntity().GetOriginAnchor().Position;
        Bot* bot = new Bot(&lootBotEntity, 0);
        bots.push_back(bot);
    }
}

CColor ClosestLightLoopFunctions::GetFloorColor(const CVector2 &c_position_on_plane) {
    return service->getColorForPosition(c_position_on_plane);
}

void ClosestLightLoopFunctions::operator()(CEntity* e)
{
    AddEntity(*e);
}

void ClosestLightLoopFunctions::createLights() {
    std::function<void(CEntity*)> fun = *this;
    service->createEntities(fun);
}

void ClosestLightLoopFunctions::setBotColorFromPercentage(Bot *bot, double state) {
    CColor color;
    if (doDebug) {
        color = getDebugColor(bot);
    } else {
//        if (bot->entity->GetId() == "fb12") {
//            color = CColor::BLUE;
//        } else
            color = (state == 0) ? CColor::BLACK : CColor(static_cast<UInt8>(255 * (1 - state)), static_cast<UInt8>(255 * state), 0);
    }
    bot->entity->GetLEDEquippedEntity().SetAllLEDsColors(color);
}

CColor ClosestLightLoopFunctions::getDebugColor(const Bot *bot) const {
    CColor color;
    string id = bot->entity->GetId();
    if (id == "fb0") {
        color = CColor::BLUE;
    } else if (id == "fb1") {
        color = CColor::RED;
    } else if (id == "fb2") {
        color = CColor::GREEN;
    } else if (id == "fb3") {
        color = CColor::YELLOW;
    } else if (id == "fb4") {
        color = CColor::GRAY50;
    } else if (id == "fb5") {
        color = CColor::ORANGE;
    } else if (id == "fb6") {
        color = CColor::MAGENTA;
    } else if (id == "fb7") {
        color = CColor::CYAN;
    } else if (id == "fb8") {
        color = CColor::BROWN;
    } else {
        color = CColor::BLACK;
    }
    return color;
}


REGISTER_LOOP_FUNCTIONS(ClosestLightLoopFunctions, "closest_light_loop_functions")
