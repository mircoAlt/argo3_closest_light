/*
 * AUTHOR: Carlo Pinciroli <cpinciro@ulb.ac.be>
 *
 * An example flocking controller for the loot-bot.
 *
 * This controller lets a group of loot-bots flock in an hexagonal lattice towards
 * a light source placed in the arena. To flock, it exploits a generalization of the
 * well known Lennard-Jones potential. The parameters of the Lennard-Jones function
 * were chosen through a simple trial-and-error procedure on its graph.
 *
 * This controller is meant to be used with the XML file:
 *    experiments/flocking.argos
 */

#ifndef CLOSEST_LIGHT_H
#define CLOSEST_LIGHT_H

/*
 * Include some necessary headers.
 */
/* Definition of the CCI_Controller class. */
static const char *const POSITIONING_SENSOR = "positioning";

static const char *const GROUND_SENSOR = "lootbot_motor_ground";

static const char *const PROXIMITY_SENSOR = "lootbot_proximity";

static const char *const LIGHT_SENSOR = "lootbot_light";

#include <argos3/core/control_interface/ci_controller.h>
/* Definition of the differential steering actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_differential_steering_actuator.h>
/* Definition of the LEDs actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_leds_actuator.h>
/* Definition of the omnidirectional camera sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_colored_blob_omnidirectional_camera_sensor.h>
/* Definition of the loot-bot light sensor */
#include <argos3/plugins/robots/loot-bot/control_interface/ci_lootbot_light_sensor.h>
/* Vector2 definitions */
#include <argos3/core/utility/math/vector2.h>
#include <argos3/plugins/robots/generic/control_interface/ci_positioning_sensor.h>
#include <loop_functions/closest_light_loop_functions/closest_light_loop_functions.h>
#include <argos3/plugins/robots/loot-bot/control_interface/ci_lootbot_motor_ground_sensor.h>
#include <argos3/plugins/robots/loot-bot/control_interface/ci_lootbot_light_sensor.h>
#include <argos3/plugins/robots/loot-bot/control_interface/ci_lootbot_proximity_sensor.h>
#include <services/readings_service.h>

/*
 * All the ARGoS stuff in the 'argos' namespace.
 * With this statement, you save typing argos:: every time.
 */
using namespace argos;
using namespace std;

/*
 * A controller is simply an implementation of the CCI_Controller class.
 */
class CLootBotClosest : public CCI_Controller {

public:
    struct SServices {
        BotsAndLightsService *forLights = nullptr;
        BotsAndLightsService *fotBotsFc = nullptr;
        BotsAndLightsService *fotBotsOa = nullptr;
        BotsAndLightsService *fotWalls = nullptr;
    } balServices;
    /*
     * The following variables are used as parameters for
     * turning during navigation. You can set their value
     * in the <parameters> section of the XML configuration
     * file, under the
     * <controllers><lootbot_flocking_controller><parameters><wheel_turning>
     * section.
     */
    struct SWheelTurningParams {
        /*
         * The turning mechanism.
         * The robot can be in three different turning states.
         */
        enum ETurningMechanism {
            NO_TURN = 0, // go straight
            HARD_TURN    // wheels are turning with opposite speeds
        } turningMechanism;
        /*
         * Angular thresholds to change turning state.
         */
        CRadians hardTurnOnAngleThreshold;
        CRadians noTurnAngleThreshold;
        /* Maximum wheel speed */
        Real maxSpeed;

        void init(TConfigurationNode &t_tree);
    };
    struct SBatteryParams {
        int randomWalkThreshold;
        int capacity;
        void init(TConfigurationNode &t_tree);
    };

    struct SArenaParams {
        int xLength;
        int yLength;
        void init(TConfigurationNode &t_tree);
    };

    bool charging;
    bool startedCharging;
    bool dead;
    unsigned long timestamp = 0;
    bool doDebug;
public:
    /* Class constructor. */
    CLootBotClosest();

    /*
     * This function initializes the controller.
     * The 't_node' variable points to the <parameters> section in the XML file
     * in the <controllers><lootbot_flocking_controller> section.
     */
    virtual void Init(TConfigurationNode &node);

    /*
     * This function is called once every time step.
     * The length of the time step is set in the XML file.
     */
    virtual void ControlStep();

    /*
     * This function resets the controller to its state right after the Init().
     * It is called when you press the reset button in the GUI.
     * In this example controller there is no need for resetting anything, so
     * the function could have been omitted. It's here just for completeness.
     */
    virtual void Reset() {}

    /*
     * Called to cleanup what done by Init() when the experiment finishes.
     * In this example controller there is no need for clean anything up, so
     * the function could have been omitted. It's here just for completeness.
     */
    virtual void Destroy() {}

    inline void setNextLight(CVector2 *position) {
        delete nextLightPosition;
        nextLightPosition = position;
    }


    inline void setNextLight(CVector2 position) {
        auto vec = new CVector2(position);
        setNextLight(vec);
        hasUpdated = lightCloserThan(furtherThreshold);
    }

    inline CVector2* getNextLightPos() {
        return nextLightPosition;
    }

    int getBatteryLevel() {
        return battery;
    }

    SBatteryParams batteryParams;

protected:

    /*
     * Calculates the vector to the closest light.
     */
    virtual CVector2 vectorToPosition(CVector2 target);

    /*
     * Gets a direction vector as input and transforms it into wheel actuation.
     */
    void setWheelSpeedsFromVector(CVector2 heading, bool stopAtObstacle = false);

private:
    const double targetThreshold = 0.1;
    const double furtherThreshold = 20 * targetThreshold;
    const int chargePerStep = 10;
    bool hasUpdated = false;
    /* Pointer to the differential steering actuator */
    CCI_DifferentialSteeringActuator *steeringActuator;
    /* Pointer to the positioning sensor */
    CCI_PositioningSensor *positioningSensor;
    CCI_LootBotMotorGroundSensor *groundSensor;
    CCI_LootBotProximitySensor *proximitySensor;
    CCI_LootBotLightSensor *lightSensor;

    CCI_PositioningSensor::SReading positioningReading;
    CCI_LootBotMotorGroundSensor::TReadings groundReadings;
    CCI_LootBotProximitySensor::TReadings proximityReadings;
    CCI_LootBotLightSensor::TReadings lightReadings;

    map<string, bool> hasRead;

    SWheelTurningParams wheelTurningParams;
    SArenaParams arenaParams;

    CVector2 *nextLightPosition;
    CRandom::CRNG* pcRNG;
    CRadians randomDirection = CRadians::ZERO;
    unsigned int randomTimer = 0;

    int battery;

    SWheelTurningParams::ETurningMechanism calculateTurningMechanism(const CRadians &angle);

    void aimToLight();

    CVector2 directionByColor(CColor *color);

    CColor colorForReading(Real reading) const;

    CColor * majorColor(vector<CColor> colors) const;

    void aimToRandomTarget(bool refreshTarget);

    void updateBattery();

    void doCharging();

    bool moveToRandomTargetSinceChargingWasCancelled();

    void turn(CRadians headingAngle);

    void stop() const;

    void getSensorsAndActuators();

    void readParams(TConfigurationNode &node);

    void refreshRandomTarget(CRadians min = CRadians::ZERO, CRadians max = CRadians::TWO_PI);

    void moveWheelsStraight(double speedFactor) const;

    CVector2 readLight();

    CCI_LootBotProximitySensor::TReadings &getProximityReading() {
        if (!hasRead[PROXIMITY_SENSOR]) {
            proximityReadings = proximitySensor->GetReadings();
            hasRead[PROXIMITY_SENSOR] = true;
        }
        return proximityReadings;
    }

    CCI_LootBotMotorGroundSensor::TReadings &getGroundReading() {
        if (!hasRead[GROUND_SENSOR]) {
            groundReadings = groundSensor->GetReadings();
            hasRead[GROUND_SENSOR] = true;
        }
        return groundReadings;
    }

    CCI_PositioningSensor::SReading &getPositioningReading() {
        if (!hasRead[POSITIONING_SENSOR]) {
            positioningReading = positioningSensor->GetReading();
            hasRead[POSITIONING_SENSOR] = true;
        }
        return positioningReading;
    }

    CCI_LootBotLightSensor::TReadings &getLightReading() {
        if (!hasRead[LIGHT_SENSOR]) {
            lightReadings = lightSensor->GetReadings();
            hasRead[LIGHT_SENSOR] = true;
        }
        return lightReadings;
    }

    CVector2 get2DPosition() {
        return { getPositioningReading().Position.GetX(), getPositioningReading().Position.GetY() };
    }

    bool needsLightUpdate();

    bool lightCloserThan(double threshold);

    void goToLight();

    bool moveByColor(const vector<CColor> &colors);
};

#endif
