link_directories(${CMAKE_BINARY_DIR}/controllers/lootbot_closest_light $ENV{ARGOS_PLUGIN_PATH})
set(closest_light_loop_functions_SOURCES closest_light_loop_functions.cpp)

add_library(closest_light_loop_functions MODULE ${closest_light_loop_functions_SOURCES})
target_link_libraries(closest_light_loop_functions
  lootbot_closest_light
  argos3core_simulator
  argos3plugin_simulator_dynamics2d
  argos3plugin_simulator_entities
  argos3plugin_simulator_lootbot
  argos3plugin_simulator_genericrobot
  argos3plugin_simulator_media
  bot
  bot_at
  light
  my_light_entity
  readings_service
  bots_and_lights_service
  global_knowledge_service
  light_service)

if(ARGOS_COMPILE_QTOPENGL)
  target_link_libraries(closest_light_loop_functions argos3plugin_simulator_qtopengl)
endif(ARGOS_COMPILE_QTOPENGL)
