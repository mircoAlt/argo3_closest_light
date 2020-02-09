#ifndef ARGOS3_EXAMPLES_MY_LIGHT_ENTITY_H
#define ARGOS3_EXAMPLES_MY_LIGHT_ENTITY_H
#include <argos3/plugins/simulator/entities/light_entity.h>
#include <models/bot.h>
#include <list>
#include <numeric>
#include <unordered_set>
#include <algorithm>
#include <set>
#include "light.h"

using namespace argos;
using namespace std;

class MyLightEntity : public Light {

    enum Axis {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

public:
    struct ColoredArea {
        CColor color;
        CRange<Real> xRange;
        CRange<Real> yRange;

        inline friend std::ostream &operator<<(std::ostream &c_os,
                                               const ColoredArea &c) {
            c_os << "Color: " << c.color << " from (" << c.xRange.GetMin() << ", " << c.yRange.GetMin() << ") to (" << c.xRange.GetMax() << ", " << c.yRange.GetMax() << ")";
            return c_os;
        }
    };

    CRange<Real> getXRange() const;
    CRange<Real> getYRange() const;

    unsigned long noOfActualBots = 0;

    void addToAssignedBots() {
        noOfActualBots++;
    }

//    void removeFromAssignedBots(Bot *bot) {
//        auto position = find(assignedBots.begin(), assignedBots.end(), *bot);
//        if (position != assignedBots.end()) {
//            assignedBots.erase(position);
//        }
////        assignedBots.erase(remove(assignedBots.begin(), assignedBots.end(), *bot), assignedBots.end());
//    }

    inline friend std::ostream &operator<<(std::ostream &c_os,
                                           const MyLightEntity &l) {

        c_os << "[L" << l.id << "] Position: " << l.position << ", axis: "<< l.axis << ", colored areas: " << l.coloredAreas.size() << ", nearest bots size : " << l.assignedBots.size();
//        for (auto *color : l.coloredAreas) {
//            c_os << endl << *color;
//        }
//        c_os << endl << ", x range " << l.getXRange() << ", y range " << l.getYRange();
        return c_os;
    }

    bool isBotNearby(Bot *bot);

    CColor* getColorForPosition(CVector2 pos);

    bool overlapsWith(MyLightEntity* otherLight);

    CLightEntity *createEntity();

    Axis axis;

    bool overlapsWith(const vector<MyLightEntity*>& otherLights);

    ColoredArea* getEntryArea();

    bool isPosInRange(CVector2 pos, CRange<Real> xRange, CRange<Real> yRange);

    MyLightEntity(CVector3 pos, int cId, double maxY, Real lightDist, CColor color);

private:

    unordered_set<string> assignedBots;
    vector<ColoredArea*> coloredAreas;
    ColoredArea* entryArea;
    Real lightDist;
    CColor color;

    ColoredArea lightArea;

    void calculateAxis(double maxY);

    void calculateColoredAreas();

    void configureArea(MyLightEntity::ColoredArea *area, CColor c, double xMinFactor, double xMaxFactor, double yMinFactor,
                       double yMaxFactor) const;

};


#endif //ARGOS3_EXAMPLES_MY_LIGHT_ENTITY_H
