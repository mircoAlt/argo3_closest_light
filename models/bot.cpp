//
// Created by Mirco Altenbernd on 16.10.19.
//

#include "bot.h"

Bot::Bot(CLootBotEntity *fromEntity, unsigned long timestamp)
        : BotAt({fromEntity->GetEmbodiedEntity().GetOriginAnchor().Position.GetX(),
                 fromEntity->GetEmbodiedEntity().GetOriginAnchor().Position.GetY()},
                         100, timestamp), entity(fromEntity) {
    id = entity->GetId();
}

void Bot::update() {
    Bot updater(entity, timestamp + 1);
    BotAt::update(updater);
}
