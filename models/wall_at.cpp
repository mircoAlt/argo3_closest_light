#include <argos3/core/utility/logging/argos_log.h>
#include "wall_at.h"

std::ostream &operator<<(std::ostream &c_os, const WallAt &c) {
    string axis = c.onX ? "X" : "Y";

    c_os << "on " << axis << " axis with value " << c.value;
    return c_os;
}

bool WallAt::operator==(const WallAt &otherWall) const {
    if (onX != otherWall.onX) {
        return false;
    }
    Real diff = value - otherWall.value;
    return Abs(diff) < 0.25;
}

bool WallAt::operator<(const WallAt &otherWall) const {
    return value < otherWall.value;
}

WallAt::WallAt(CVector2 position, bool isOnX) {
    onX = isOnX;
    value = isOnX ? position.GetY() : position.GetX();
}
