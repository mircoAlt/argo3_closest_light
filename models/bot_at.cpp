#include "bot_at.h"
#import <services/debug_log.h>
void BotAt::update(const BotAt& fromBot) {
    CVector2 diff = fromBot.position - position;
    if (noOfReadings == fromBot.noOfReadings && noOfReadings != 0) {
        if(diff.Length() > 0.005) {
            movesPerTs = diff / (fromBot.timestamp - timestamp);
        } else {
            movesPerTs = CVector2();
        }
    }

    position = fromBot.position;
    doOaUpdate(fromBot);
    noOfReadings = fromBot.noOfReadings;
    timestamp = fromBot.timestamp;
//    if (bot != nullptr && bot->entity.GetId() == "fb5") cout << *this << endl;
}

CVector2 BotAt::assumedPosition(unsigned long currentTime, bool forOa) const {
    unsigned long timeDiff = currentTime - timestamp;
    if (forOa && timeDiff <= 1) {
        return oaPosition;
    }
    CVector2 movedSince = (timeDiff > 0) ? movesPerTs * timeDiff : CVector2();
    return (forOa ? oaPosition : position ) + movedSince;

}

bool BotAt::operator==(const BotAt &otherBot) const {
    long timeDiff = Abs(((long) timestamp) - (long) otherBot.timestamp);
    if (timeDiff > 100) {
        return false;
    }
    double diffToOther = (position - otherBot.position).Length();
    bool returnValue = diffToOther < (double) timeDiff / 4.0;
    return returnValue;
}

void BotAt::doOaUpdate(const BotAt &fromBot) {
    if (fromBot.timestamp - timestamp > 1) {
        oaPosition = position;
        return;
    }
    if (oas.size() == 5) {
        oaPosition = {};
        for (auto &oa: oas) {
            oaPosition += oa;
        }
        oaPosition /= 5;
        oas = {};
    } else {
        oas.push_back(position);
    }
}
