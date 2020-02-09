/* Include the controller definition */
#include <argos3/core/utility/math/rng.h>
#include <services/debug_log.h>
#include "lootbot_closest_light.h"

CLootBotClosest::CLootBotClosest() :
        steeringActuator(nullptr),
        charging(false),
        startedCharging(false),
        dead(false),
        pcRNG(nullptr),
        nextLightPosition(nullptr) {}


void CLootBotClosest::Init(TConfigurationNode &node) {
    getSensorsAndActuators();
    pcRNG = CRandom::CreateRNG("argos");

    try {
        readParams(node);
    }
    catch (CARGoSException &ex) {
        THROW_ARGOSEXCEPTION_NESTED("Error parsing the controller parameters.", ex);
    }
    refreshRandomTarget();
    battery = pcRNG->Uniform(CRange<int>(batteryParams.randomWalkThreshold, batteryParams.capacity));
}

void CLootBotClosest::ControlStep() {
    hasRead[PROXIMITY_SENSOR] = false;
    hasRead[GROUND_SENSOR] = false;
    hasRead[POSITIONING_SENSOR] = false;
    hasRead[LIGHT_SENSOR] = false;
    startedCharging = false;
    timestamp++;
    auto positionOfBot = getPositioningReading();
    auto proxReading = getProximityReading();
    balServices.fotWalls->addWalls(proxReading, positionOfBot, timestamp, GetId());
    balServices.fotBotsFc->addBots(proxReading, positionOfBot, timestamp, GetId(), balServices.fotWalls);
    if (balServices.fotBotsOa != nullptr)
        balServices.fotBotsOa->addBots(proxReading, positionOfBot, timestamp, GetId(), balServices.fotWalls);
    if (!dead) {
        if (nextLightPosition != nullptr) {
           if (needsLightUpdate()) {
               setNextLight(readLight());
            }
            aimToLight();
        } else {
            aimToRandomTarget(randomTimer <= 0);
            randomTimer--;
        }
        updateBattery();
    } else {
        aimToLight();
    }

}

bool CLootBotClosest::needsLightUpdate() {
    if (!hasUpdated && lightCloserThan(furtherThreshold)) {
        return true;
    }
    return lightCloserThan(4 * targetThreshold);
}

bool CLootBotClosest::lightCloserThan(double threshold) {
    return nextLightPosition != nullptr && (*nextLightPosition - get2DPosition()).Length() <= threshold;
}

bool CLootBotClosest::moveToRandomTargetSinceChargingWasCancelled() {
    if (charging) {
        RLOG << "charging aborted" << endl;
        charging = false;
        Real cap = batteryParams.randomWalkThreshold + (batteryParams.capacity - batteryParams.randomWalkThreshold) / 2.0;
        if (battery > cap) {
            RLOG << "giving up" << endl;
            setNextLight(nullptr);
            return true;
        }
        RLOG << "retry" << endl;
    }
    return false;
}

void CLootBotClosest::doCharging() {
    if (!charging) {
        charging = true;
        startedCharging = true;
    }
    battery += chargePerStep;
    int goal = batteryParams.capacity
//            - (0.1 * batteryParams.capacity * balServices.fotBotsFc->noOfBotsNearby(get2DPosition(), GetId(), timestamp))
            ;
    if (battery >= goal) {
        RLOG << "finished charging" << endl;
        setNextLight(nullptr);
        charging = false;
        aimToRandomTarget(true);
    } else {
        stop();
    }
}

void CLootBotClosest::aimToRandomTarget(bool refreshTarget) {
    if (refreshTarget) {
        refreshRandomTarget();
    }
    if (balServices.fotWalls->isAimingAtWall(getPositioningReading(), randomDirection)) {
        BOTLOG << "aiming at wall" << endl;
        refreshRandomTarget(randomDirection + CRadians::PI_OVER_TWO, randomDirection + 3 *  CRadians::PI_OVER_TWO);
    }
//    if (battery / (double) batteryParams.capacity > 0.95) {
//        vector<CColor> colors;
//        for (int i = 0; i < 4; i++) {
//            auto color = colorForReading(getGroundReading()[i].Value);
//            colors.push_back(color);
//        }
//        if (moveByColor(colors)) {
//            return;
//        }
//    }
//    if (battery / (double) batteryParams.capacity > 0.9) {
//        moveWheelsStraight(1);
//        return;
//    }
    CVector2 randomTarget = get2DPosition() + CVector2(10, randomDirection);
    setWheelSpeedsFromVector(vectorToPosition(randomTarget));
}

void CLootBotClosest::updateBattery() {
    battery--;
    if (battery == 0) {
        stop();
        RLOG << "Battery died" << endl;
        dead = true;
    }

    if (battery == batteryParams.randomWalkThreshold) {
        setNextLight(readLight());
    }
}

CVector2 CLootBotClosest::readLight() {
    auto position = getPositioningReading();
    auto lightReads = getLightReading();
    auto result = balServices.forLights->calculateRelevantLight(lightReads, position, nextLightPosition, GetId(),
                                                                balServices.fotBotsFc, timestamp);
    if (result.getId() == -1) {
        return (nextLightPosition != nullptr) ? *nextLightPosition : CVector2();
    } else {
        return result.get2DPosition();
    }
}

void CLootBotClosest::aimToLight() {
    if (lightCloserThan(furtherThreshold)) {
        auto groundReads = getGroundReading();
        vector<CColor> colors;
        for (int i = 0; i < 4; i++) {
            auto color = colorForReading(groundReads[i].Value);
            colors.push_back(color);
            if (color == CColor::GRAY80) {
                // bot is atLight and should be charging
                doCharging();
                return;
            }
        }

        if (dead) {
            return;
        }

        if (moveToRandomTargetSinceChargingWasCancelled()) {
            aimToRandomTarget(true);
            return;
        }

        if (!moveByColor(colors)) {
            goToLight();
        }
    } else if (!dead) {
        goToLight();
    }

}

bool CLootBotClosest::moveByColor(const vector<CColor> &colors) {
    CColor *readColor = majorColor(colors);
    if (readColor == nullptr) {
        // bot is on different colors; continue straight
        moveWheelsStraight(1);
        return true;
    }
    if (*readColor == CColor::WHITE) {
        return false;
    }
    // bot is in colored area
    setWheelSpeedsFromVector(directionByColor(readColor), true);
    return true;
}

void CLootBotClosest::goToLight() {
    if (nextLightPosition == nullptr) {
        stop();
        RLOG << "Could not get light" << endl;
    } else {
        // bot is far away from the light
        setWheelSpeedsFromVector(vectorToPosition(*nextLightPosition));
    }
}

CColor * CLootBotClosest::majorColor(vector<CColor> colors) const {
    for (int i = 1; i < colors.size(); i++) {
        if (colors[i] != colors[0]) {
            return nullptr;
        }
    }
    return &colors[0];
}

CColor CLootBotClosest::colorForReading(Real reading) const {
    CColor readColor;
    if (reading > 0.9) {
        readColor = CColor::WHITE;
    } else if (reading > 0.75) {
        readColor = CColor::GRAY80;
    } else if (reading > 0.55) {
        readColor = CColor::GRAY60;
    } else if (reading > 0.35) {
        readColor = CColor::GRAY40;
    } else if (reading > 0.1) {
        readColor = CColor::GRAY20;
    } else {
        readColor = CColor::BLACK;
    }
    return readColor;
}


CVector2 CLootBotClosest::vectorToPosition(CVector2 target) {
    auto tProx = getPositioningReading();
    CQuaternion inv = tProx.Orientation.Inverse();
    CVector3 toTarget = CVector3(target.GetX(), target.GetY(), 0) - tProx.Position;
    CVector3 toTargetBotSpace = toTarget.Rotate(inv);
    CVector2 toTargetBotSpace2D = CVector2(toTargetBotSpace.GetX(), toTargetBotSpace.GetY());
    if (isnan(toTargetBotSpace2D.GetX()) || isnan(toTargetBotSpace2D.GetY())) {
        RLOGERR << "could not convert target " << target << endl;
        RLOGERR << "orientation " << tProx.Orientation << endl;
        RLOGERR << "position " << tProx.Position << endl;
        return {};
    }
    return toTargetBotSpace2D;

}

CVector2 CLootBotClosest::directionByColor(CColor *color) {
    CVector2 direction;
    if (*color == CColor::GRAY20) {
        direction = { 0,-1 };
    } else if (*color == CColor::GRAY40) {
        direction = { -1,0 };
    } else if (*color == CColor::GRAY60) {
        direction = { 0,1 };
    } else if(*color == CColor::BLACK) {
        direction = { 1,0 };
    }
    CVector2 aimTo = get2DPosition() + direction;
    return vectorToPosition(aimTo);
}

void CLootBotClosest::setWheelSpeedsFromVector(CVector2 heading, bool stopAtObstacle) {
    if (heading == CVector2::ZERO) {
        stop();
        return;
    }
    bool isClose = lightCloserThan(10* targetThreshold);
    auto botService = (balServices.fotBotsOa != nullptr) ? balServices.fotBotsOa : balServices.fotBotsFc;
    if (isnan(heading.GetX()) || isnan(heading.GetY())) {
        RLOGERR << "heading is nan" << endl;
        stop();
        return;
    }
    auto direction = balServices.fotWalls->getClosestFreeAngle(heading,
                                                               getPositioningReading(), timestamp,
                                                               nextLightPosition != nullptr,
                                                               GetId(), botService);
//    if (Abs(NormalizedDifference(direction.first, heading.Angle())) > CRadians(0.05)) {
//        BOTLOG << "original angle: " << heading.Angle() << ", going to: " << direction.first << endl;
//    }
//    CRadians *angle = stopAtObstacle || isClose ? new CRadians(headingAngle) : readingsService->getClosestFreeAngle(headingAngle, getPositioningReading(), timestamp);
    if (direction.second == 0.0) {
        RLOG << "cannot move" << endl;
        stop();
        return;
    }
    wheelTurningParams.turningMechanism = calculateTurningMechanism(direction.first);
    switch (wheelTurningParams.turningMechanism) {
        case SWheelTurningParams::NO_TURN: {
            moveWheelsStraight(direction.second);
            break;
        }
        case SWheelTurningParams::HARD_TURN:
            turn(direction.first);
            break;
    }
}

void CLootBotClosest::moveWheelsStraight(double speedMultiplier) const {
    steeringActuator->SetLinearVelocity(speedMultiplier * wheelTurningParams.maxSpeed,
                                        speedMultiplier * wheelTurningParams.maxSpeed);
}

void CLootBotClosest::stop() const { steeringActuator->SetLinearVelocity(0, 0); }

void CLootBotClosest::turn(CRadians headingAngle) {
    Real leftWheelSpeed, rightWheelSpeed;
    /* Opposite wheel speeds */
    rightWheelSpeed = headingAngle.GetValue() * wheelTurningParams.maxSpeed;
    leftWheelSpeed = -rightWheelSpeed;
    steeringActuator->SetLinearVelocity(leftWheelSpeed, rightWheelSpeed);

}

void CLootBotClosest::refreshRandomTarget(CRadians min, CRadians max) {
    randomDirection = CRadians(pcRNG->Uniform(CRange<Real>(min.GetValue(), max.GetValue())));
    UInt32 diff = batteryParams.capacity - batteryParams.randomWalkThreshold;
    randomTimer = pcRNG->Uniform(CRange<UInt32>(0, diff));
}

void CLootBotClosest::readParams(TConfigurationNode &node) {
    wheelTurningParams.init(GetNode(node, "wheel_turning"));
    batteryParams.init(GetNode(node, "battery"));
    arenaParams.init(GetNode(node, "arena"));
}

void CLootBotClosest::getSensorsAndActuators() {
    steeringActuator = GetActuator<CCI_DifferentialSteeringActuator>("differential_steering");

    positioningSensor = GetSensor<CCI_PositioningSensor>(POSITIONING_SENSOR);
    groundSensor = GetSensor<CCI_LootBotMotorGroundSensor>(GROUND_SENSOR);
    proximitySensor = GetSensor<CCI_LootBotProximitySensor>(PROXIMITY_SENSOR);
    lightSensor = GetSensor<CCI_LootBotLightSensor>(LIGHT_SENSOR);
}

CLootBotClosest::SWheelTurningParams::ETurningMechanism CLootBotClosest::calculateTurningMechanism(
        const CRadians &angle) {
    if (Abs(angle) <= wheelTurningParams.noTurnAngleThreshold) {
        return SWheelTurningParams::NO_TURN;
    } else {
        return SWheelTurningParams::HARD_TURN;
    }
}

void CLootBotClosest::SWheelTurningParams::init(TConfigurationNode &t_node) {
    try {
        turningMechanism = NO_TURN;
        CDegrees cAngle;
        GetNodeAttribute(t_node, "hard_turn_angle_threshold", cAngle);
        hardTurnOnAngleThreshold = ToRadians(cAngle);
        GetNodeAttribute(t_node, "no_turn_angle_threshold", cAngle);
        noTurnAngleThreshold = ToRadians(cAngle);
        GetNodeAttribute(t_node, "max_speed", maxSpeed);
    }
    catch (CARGoSException &ex) {
        THROW_ARGOSEXCEPTION_NESTED("Error initializing controller wheel turning parameters.", ex);
    }
}

void CLootBotClosest::SBatteryParams::init(TConfigurationNode &t_tree) {
    try {
        GetNodeAttribute(t_tree, "random_walk_threshold", randomWalkThreshold);
        GetNodeAttribute(t_tree, "capacity", capacity);
    }
    catch (CARGoSException &ex) {
        THROW_ARGOSEXCEPTION_NESTED("Error initializing controller wheel battery parameters.", ex);
    }
}

void CLootBotClosest::SArenaParams::init(TConfigurationNode &t_tree) {
    try {
        GetNodeAttribute(t_tree, "xLength", xLength);
        GetNodeAttribute(t_tree, "yLength", yLength);
    }
    catch (CARGoSException &ex) {
        THROW_ARGOSEXCEPTION_NESTED("Error initializing controller arena battery parameters.", ex);
    }
}

/*
 * This statement notifies ARGoS of the existence of the controller.
 * It binds the class passed as first argument to the string passed as second argument.
 * The string is then usable in the XML configuration file to refer to this controller.
 * When ARGoS reads that string in the XML file, it knows which controller class to instantiate.
 * See also the XML configuration files for an example of how this is used.
 */
REGISTER_CONTROLLER(CLootBotClosest, "lootbot_closest_light_controller")
