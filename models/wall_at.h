#ifndef ARGOS3_EXAMPLES_WALL_AT_H
#define ARGOS3_EXAMPLES_WALL_AT_H

#include <argos3/plugins/robots/loot-bot/control_interface/ci_lootbot_light_sensor.h>
#include <argos3/plugins/robots/loot-bot/control_interface/ci_lootbot_proximity_sensor.h>
#include <argos3/core/utility/math/vector2.h>
#include <argos3/plugins/robots/generic/control_interface/ci_positioning_sensor.h>

using namespace std;
using namespace argos;
class WallAt {

public:
    bool onX;
    Real value;

    friend std::ostream &operator<<(std::ostream &c_os,
                                           const WallAt &c);

    bool operator==(const WallAt& otherWall) const;
    bool operator<(const WallAt& otherWall) const;

    Real operator -(const CVector2 otherPosition) const {
        Real relevantCoordinate = onX ? otherPosition.GetY() : otherPosition.GetX();
        return Abs(value - relevantCoordinate);
    }
    explicit WallAt(CVector2 position, bool isOnX);

    CRadians angleFrom(CVector2 position) const {
        Real relevantCoordinate = onX ? position.GetY() : position.GetX();
        if (relevantCoordinate < value) {
            return onX ? CRadians::PI_OVER_TWO : CRadians::ZERO;
        } else {
            return onX ? -CRadians::PI_OVER_TWO : CRadians::PI;
        }
    }
};


#endif //ARGOS3_EXAMPLES_WALL_AT_H
