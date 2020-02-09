//
// Created by Mirco Altenbernd on 16.10.19.
//

#ifndef ARGOS3_EXAMPLES_BOT_H
#define ARGOS3_EXAMPLES_BOT_H

#include <argos3/core/simulator/loop_functions.h>
#include <argos3/plugins/robots/loot-bot/simulator/lootbot_entity.h>
#include "bot_at.h"

using namespace argos;
using namespace std;
class Bot : public BotAt {

public:

    Bot(CLootBotEntity* fromEntity, unsigned long timestamp);
    CLootBotEntity* entity;

    inline bool operator==(const Bot &bot) const {
        return entity->GetId() == bot.entity->GetId();
    }

    void update();

    int *diedAt;
};


#endif //ARGOS3_EXAMPLES_BOT_H
