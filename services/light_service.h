#ifndef ARGOS3_EXAMPLES_LIGHTSERVICE_H
#define ARGOS3_EXAMPLES_LIGHTSERVICE_H

#include <models/my_light_entity.h>
#include <models/bot.h>
#include <functional>

using namespace std;
class LightService {

public:
    struct SLightParams {
        int noOfLights;
        Real arenaSideXLength;
        Real arenaSideYLength;
        Real lightDist;
    };

    explicit LightService(SLightParams lightParams);

    void printLights();
//    Light * calculateRelevantLight(Bot *bot);

    inline string GetId() const {
        return "light service";
    }
    string printDistribution();

    map<int, double> calculateDistances(CVector3 position);

    CColor getColorForPosition(CVector2 position);

    void createEntities(const std::function<void(CEntity*)>& fun);

    MyLightEntity *findLightAt(CVector2 position);
    const vector<MyLightEntity*> & getAllLights() const;

    SLightParams lightParams{};
private:
    vector<MyLightEntity*> allLights;

    MyLightEntity * createRandomLight(CRandom::CRNG *rng, int id) const;

    void createDebugLights(int counter);
};



#endif //ARGOS3_EXAMPLES_LIGHTSERVICE_H
