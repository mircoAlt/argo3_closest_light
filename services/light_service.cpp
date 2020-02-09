#include "light_service.h"

LightService::LightService(SLightParams lightParams): lightParams(lightParams) {
    CRandom::CRNG *rng = CRandom::CreateRNG("argos");
    int counter = 0;
    if (lightParams.noOfLights == -1) {
        createDebugLights(counter);
    } else
        while (allLights.size() != lightParams.noOfLights) {
            MyLightEntity* light = createRandomLight(rng, counter);
            if (light->overlapsWith(allLights)) {
                RLOG << "light " << light->getId() << " overlaps with existing light" << endl;
                delete light;
            }
            else {
                allLights.push_back(light);
                counter++;
            }
            if (counter == 2) {
                rng->Bernoulli();
                rng->Bernoulli();
            }
        }
}

void LightService::createDebugLights(int counter) {
    while (counter < 3) {
        Real onX, onY;
        Real maxY = lightParams.arenaSideYLength / 2.0;
        CColor color;
        if (counter == 0) {
            onX = 3.25723;
            onY = -9.775;
            color = CColor::PURPLE;
        }
        if (counter == 1) {
            onX = -4.775;
            onY = -1.78536;
            color = CColor::GREEN;
        }
        if (counter == 2) {
            onX = 4.775;
            onY = -2.59889;
            color = CColor::YELLOW;
        }
        MyLightEntity* light = new MyLightEntity(CVector3(onX, onY, 0.1), counter, maxY, lightParams.lightDist, color);
        allLights.push_back(light);

        counter++;
    }
}


MyLightEntity* LightService::findLightAt(CVector2 position) {
    auto pos2D = CVector2(position.GetX(), position.GetY());
    for (auto *l: allLights) {
        if (l->isPosInRange(pos2D, l->getXRange(), l->getYRange())) {
            return l;
        }
    }
    return nullptr;
}

MyLightEntity * LightService::createRandomLight(CRandom::CRNG *rng, int id) const {
    double chanceOfX = lightParams.arenaSideXLength /
            (lightParams.arenaSideXLength + lightParams.arenaSideYLength);
    bool placeOnXAxis = rng->Bernoulli(chanceOfX);
    bool inverse = rng->Bernoulli();
    Real maxX = (lightParams.arenaSideXLength / 2.0) - lightParams.lightDist;
    Real maxY = (lightParams.arenaSideYLength / 2.0) - lightParams.lightDist;
    Real onX, onY;
    Real avoidCorners = 7.5 * lightParams.lightDist;
    if (placeOnXAxis) {
        CRange<Real> xRange = {-maxX + avoidCorners, maxX - avoidCorners};
        onX = rng->Uniform(xRange);
        onY = maxY - lightParams.lightDist;
        if (inverse) onY *= -1;
    } else {
        CRange<Real> yRange = {-maxY + avoidCorners, maxY - avoidCorners};
        onY = rng->Uniform(yRange);
        onX = maxX - lightParams.lightDist;
        if (inverse) onX *= -1;
    }
    auto* light = new MyLightEntity(CVector3(onX, onY, 1.2), id, maxY, lightParams.lightDist, CColor::YELLOW);
    return light;
}

void LightService::printLights() {
    RLOG << "lights: " << endl;
    for (auto *light : allLights) {
        RLOG << *light << endl;
    }
    RLOG << endl;
}

string LightService::printDistribution() {
    vector<double> numbers;
    for (int i = 0; i < allLights.size(); i++) {
        RLOG << "Light (" << i << "): getting " << allLights[i]->noOfActualBots << std::endl;
        numbers.push_back(1.0 * allLights[i]->noOfActualBots);
    }
    auto mAndS = Light::meanAndStDev(numbers);
    return to_string(mAndS.first) + "; " +
            to_string(mAndS.second);
}

map<int, double> LightService::calculateDistances(CVector3 position) {
    map<int,double> distances;
    for (auto *light : allLights) {
        distances[light->getId()] = (light->getPosition() - position).Length();
    }
    return distances;
}

CColor LightService::getColorForPosition(const CVector2 position) {
    for (auto *light : allLights) {
        CColor *lightColor = light->getColorForPosition(position);
        if (lightColor != nullptr
           // && *lightColor == CColor::GRAY80
        ) {
            return *lightColor;
        }
    }
    return CColor::WHITE;
}

void LightService::createEntities(const std::function<void(CEntity *)>& fun) {
    for (auto *light: allLights) {
        fun(light->createEntity());
    }

}

//void LightService::addBotToNearestLight(Bot *bot) {
//    int minId = bot->getNearestLightId();
//    MyLightEntity *minLight = allLights[minId];
//    minLight->addToAssignedBots();
//}

const vector<MyLightEntity*> & LightService::getAllLights() const {
    return allLights;
}
