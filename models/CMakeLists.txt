add_library(bot STATIC bot.h bot.cpp)
add_library(bot_at STATIC bot_at.h bot_at.cpp)
add_library(wall_at STATIC wall_at.h wall_at.cpp)
add_library(my_light_entity STATIC my_light_entity.h my_light_entity.cpp)
add_library(light STATIC light.h light.cpp)
target_link_libraries(my_light_entity light bot bot_at wall_at
        argos3core_simulator
        argos3plugin_simulator_entities
        argos3plugin_simulator_lootbot
        argos3plugin_simulator_genericrobot)