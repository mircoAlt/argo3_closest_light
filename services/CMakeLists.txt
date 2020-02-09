add_library(light_service STATIC light_service.h light_service.cpp debug_log.h)
add_library(readings_service STATIC readings_service.cpp readings_service.h debug_log.h score_wheel.cpp score_wheel.h global_knowledge_service.cpp global_knowledge_service.h)
add_library(bots_and_lights_service STATIC bots_and_lights_service.cpp bots_and_lights_service.h)
add_library(global_knowledge_service STATIC global_knowledge_service.cpp global_knowledge_service.h)
target_link_libraries(light_service
        bots_and_lights_service
        global_knowledge_service
        readings_service
        bot
        bot_at
        wall_at
        my_light_entity
        argos3core_simulator
        argos3plugin_simulator_entities
        argos3plugin_simulator_lootbot
        argos3plugin_simulator_genericrobot)