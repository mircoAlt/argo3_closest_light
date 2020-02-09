#ifndef CLOSEST_LIGHT_LOOP_FUNCTIONS_H
#define CLOSEST_LIGHT_LOOP_FUNCTIONS_H

#include <argos3/core/simulator/loop_functions.h>
#include <argos3/core/simulator/entity/floor_entity.h>
#include <argos3/core/utility/math/range.h>
#include <argos3/core/utility/math/rng.h>
#include <argos3/plugins/robots/loot-bot/simulator/lootbot_entity.h>
#include <argos3/plugins/simulator/entities/proximity_sensor_equipped_entity.h>
#include <argos3/plugins/simulator/entities/light_entity.h>
#include <list>
#include <numeric>
#include <unordered_set>
#include <models/my_light_entity.h>
#include <models/bot.h>
#include <services/light_service.h>
#include <services/global_knowledge_service.h>

using namespace argos;
using namespace std;

class ClosestLightLoopFunctions : public CLoopFunctions {

public:
    enum CriteriaMode {
        BOTS_LOCAL = 0,
        BOTS_GLOBAL = 1,
        CLOSEST = 2
    };

    struct SGlobalLocalConfig {
        bool lights = false;
        CriteriaMode botsFc = CriteriaMode::BOTS_LOCAL;
        bool botsOa = false;
        bool walls = false;
        bool needsGlobalKnowledge() const {
            return lights || botsFc != CriteriaMode::BOTS_LOCAL || botsOa || walls;
        }

        bool needsReadingService() const {
            return !lights || botsFc == CriteriaMode::BOTS_LOCAL || !botsOa || !walls;
        }
        inline friend std::ostream &operator<<(std::ostream &c_os,
                                               const SGlobalLocalConfig &c) {

            c_os << c.lights << c.botsFc << c.botsOa << c.walls;
            return c_os;
        }
        SGlobalLocalConfig() = default;
    };
    SGlobalLocalConfig glConfig;
    ClosestLightLoopFunctions();

    virtual void Init(TConfigurationNode &node);
    virtual void PreStep();
    virtual void PostStep();
    virtual void Destroy();
    virtual void Reset();

    virtual bool IsExperimentFinished();

    virtual void PostExperiment();

    virtual CColor GetFloorColor(const CVector2& c_position_on_plane);
    void operator()(CEntity* e);

private:
    bool doDebug = false;
    vector<Bot*> bots;

    unordered_set<string> finishedBots;

    LightService* service;

    GlobalKnowledgeService *gkService;

    void initBots();

    inline string GetId() const {
        return "closest light controller";
    }

    void createLights();

    void setBotColorFromPercentage(Bot *bot, double state);

    LightService::SLightParams readLightParams(TConfigurationNode &node);

    SGlobalLocalConfig readGlParams(TConfigurationNode &node);

    CColor getDebugColor(const Bot *bot) const;
};

#endif
