#ifndef ARGOS3_EXAMPLES_BOT_AT_H
#define ARGOS3_EXAMPLES_BOT_AT_H

#include <argos3/plugins/robots/loot-bot/control_interface/ci_lootbot_light_sensor.h>
#include <argos3/plugins/robots/loot-bot/control_interface/ci_lootbot_proximity_sensor.h>
#include <argos3/core/utility/math/vector2.h>
#include <argos3/core/utility/math/rng.h>
#include <argos3/plugins/robots/generic/control_interface/ci_positioning_sensor.h>
#include <models/light.h>

using namespace argos;
using namespace std;
class BotAt {

public:
    constexpr static const double radiusOfRobot = 0.305966;
    CVector2 oaPosition;
    unsigned long timestamp = 0;
//    Bot *bot = nullptr;
    Light *closestLight{};
    CVector2 movesPerTs;
    int noOfReadings = 0;

    bool operator==(const BotAt& otherBot) const;

    void update(const BotAt& fromBot);

    inline string GetId() const {
        return id;
    }

    CVector2 assumedPosition(unsigned long currentTime, bool forOa = false) const;

    inline friend std::ostream &operator<<(std::ostream &c_os,
                                           const BotAt &b) {
        c_os << "[" << b.GetId() << "] Position: " << b.position << " mpts " << b.movesPerTs << " no of readings" << b.noOfReadings;
            c_os << ", oaPosition: " << b.oaPosition;
        return c_os;
    }

    BotAt(CVector2 position, int noOfReadings,  unsigned long timestamp ):
        position(position), oaPosition(position), noOfReadings(noOfReadings), timestamp(timestamp), oaTs(timestamp) {
    }

protected:
    string id = "unknown bot";
private:
    CVector2 position;
    vector<CVector2> oas;
    unsigned long oaTs;
    void doOaUpdate(const BotAt& fromBot);
};


#endif //ARGOS3_EXAMPLES_BOT_AT_H
