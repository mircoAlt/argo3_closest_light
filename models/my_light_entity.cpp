#include "my_light_entity.h"

MyLightEntity::MyLightEntity(CVector3 pos, int cId, double maxY, Real lightDist, CColor color) : Light(pos, cId), color(color) {
    this->lightDist = lightDist;
    calculateAxis(maxY);
    lightArea = ColoredArea();
    lightArea.color = CColor::GRAY80;
    lightArea.xRange = { pos.GetX() - lightDist, pos.GetX() + lightDist};
    lightArea.yRange = { pos.GetY() - lightDist, pos.GetY() + lightDist};
    coloredAreas.push_back(&lightArea);
    calculateColoredAreas();
}

CRange<Real> MyLightEntity::getXRange() const {
    auto noOfColoredAreas = static_cast<int>(coloredAreas.size());
    Real allMins[noOfColoredAreas], allMaxs[noOfColoredAreas];
    for (int i = 0; i < noOfColoredAreas; i++) {
        allMins[i] = coloredAreas[i]->xRange.GetMin();
        allMaxs[i] = coloredAreas[i]->xRange.GetMax();
    }

    return { *min_element(allMins, allMins + noOfColoredAreas),
             *max_element(allMaxs, allMaxs + noOfColoredAreas)};
}
CRange<Real> MyLightEntity::getYRange() const {
    auto noOfColoredAreas = static_cast<int>(coloredAreas.size());
    Real allMins[noOfColoredAreas], allMaxs[noOfColoredAreas];
    for (int i = 0; i < noOfColoredAreas; i++) {
        allMins[i] = coloredAreas[i]->yRange.GetMin();
        allMaxs[i] = coloredAreas[i]->yRange.GetMax();
    }

    return {*min_element(allMins, allMins + noOfColoredAreas),
            *max_element(allMaxs, allMaxs + noOfColoredAreas)};
}

bool MyLightEntity::overlapsWith(MyLightEntity* otherLight) {
    bool xOverlap = otherLight->getXRange().WithinMinBoundIncludedMaxBoundIncluded(getXRange().GetMax())
            || getXRange().WithinMinBoundIncludedMaxBoundIncluded(otherLight->getXRange().GetMax());

    bool yOverlap = otherLight->getYRange().WithinMinBoundIncludedMaxBoundIncluded(getYRange().GetMax())
            || getYRange().WithinMinBoundIncludedMaxBoundIncluded(otherLight->getYRange().GetMax());

    return xOverlap && yOverlap;
}

bool MyLightEntity::overlapsWith(const vector<MyLightEntity*>& otherLights) {
    for (auto &otherLight: otherLights) {
        if (overlapsWith(otherLight)) {
            return true;
        }
    }
    return false;
}


void MyLightEntity::calculateAxis(double maxY) {
    bool onX = Abs(position.GetY()) >= (maxY - lightDist);
    if (onX) {
        axis = position.GetY() < 0 ? Axis::DOWN : Axis::UP;
    } else {
        axis = position.GetX() < 0 ? Axis::LEFT : Axis::RIGHT;
    }

}

void MyLightEntity::calculateColoredAreas() {
    auto red = new ColoredArea();
    auto blue = new ColoredArea();
    auto green = new ColoredArea();
    auto yellow = new ColoredArea();
    vector<ColoredArea*> newAreas;
    CColor redColor = CColor::BLACK;
    CColor blueColor = CColor::GRAY20;
    CColor greenColor = CColor::GRAY40;
    CColor yellowColor = CColor::GRAY60;
    double length = 7.5;
    double width = 5.0;
    double centerWidth = 1.0 - width;
    double lowerWidth = centerWidth - width;
    int lowerDiff = 2;

    switch (axis) {
        case Axis::UP :
            newAreas = {blue, green, yellow};
            configureArea(blue, blueColor, 0, length, centerWidth, lowerDiff);
            configureArea(green, greenColor, -lowerDiff, length, lowerWidth, centerWidth);
            configureArea(yellow, yellowColor, -length, -lowerDiff, lowerWidth, centerWidth);
            break;
        case Axis::RIGHT :
            newAreas = {red, green, yellow};
            configureArea(green, greenColor, centerWidth, lowerDiff, -length, 0);
            configureArea(yellow, yellowColor, lowerWidth, centerWidth, -length, lowerDiff);
            configureArea(red, redColor, lowerWidth, centerWidth, lowerDiff, length);
            break;
        case DOWN:
            newAreas = {red, blue, yellow};
            configureArea(yellow, yellowColor, -length, 0, -lowerDiff, -centerWidth);
            configureArea(red, redColor, -length, lowerDiff, -centerWidth, -lowerWidth);
            configureArea(blue, blueColor, lowerDiff, length, -centerWidth, -lowerWidth);
            break;
        case LEFT:
            newAreas = {red, blue, green};
            configureArea(red, redColor, -lowerDiff, -centerWidth, 0, length);
            configureArea(blue, blueColor, -centerWidth, -lowerWidth, -lowerDiff, length);
            configureArea(green, greenColor, -centerWidth, -lowerWidth, -length, -lowerDiff);
            break;
    }
    coloredAreas.insert(coloredAreas.end(), newAreas.begin(), newAreas.end());

}

void MyLightEntity::configureArea(MyLightEntity::ColoredArea *area, CColor c, double xMinFactor, double xMaxFactor, double yMinFactor,
                                  double yMaxFactor) const {
    area->color = c;
    area->xRange = {position.GetX() + (xMinFactor * lightDist), position.GetX() + (xMaxFactor * lightDist) };
    area->yRange = {position.GetY() + (yMinFactor * lightDist), position.GetY() + (yMaxFactor*lightDist) };
}


CLightEntity *MyLightEntity::createEntity() {
    return new CLightEntity(
            "ld" + to_string(id),
            position,
            color,
            7.0);
}

bool MyLightEntity::isBotNearby(Bot *bot) {
    return find(assignedBots.begin(), assignedBots.end(), bot->entity->GetId()) != assignedBots.end();
}

CColor *MyLightEntity::getColorForPosition(CVector2 pos) {
//    for (auto *area : coloredAreas) {
//        if (lightArea.xRange.WithinMinBoundIncludedMaxBoundExcluded(pos.GetX()) && lightArea.yRange.WithinMinBoundIncludedMaxBoundExcluded(pos.GetY())){
//            return &lightArea.color;
//        }
//    }
    for (auto *area : coloredAreas) {
        if (isPosInRange(pos, area->xRange, area->yRange)){
            return &area->color;
        }
    }
    return nullptr;
}

bool MyLightEntity::isPosInRange(CVector2 pos, CRange<Real> xRange, CRange<Real> yRange) {
    return xRange.WithinMinBoundIncludedMaxBoundExcluded(pos.GetX()) && yRange.WithinMinBoundIncludedMaxBoundExcluded(pos.GetY());
}

MyLightEntity::ColoredArea *MyLightEntity::getEntryArea() {
    return entryArea;
}
