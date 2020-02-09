add_library(lootbot_closest_light SHARED lootbot_closest_light.h lootbot_closest_light.cpp)
target_link_libraries(lootbot_closest_light
  argos3core_simulator
  argos3plugin_simulator_lootbot
  argos3plugin_simulator_genericrobot
  readings_service
  bots_and_lights_service
  bot
  light
  bot_at
  wall_at)
