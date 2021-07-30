cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if(NOT TARGET score_lib_base)
  include("${SCORE_SOURCE_DIR}/cmake/ScoreExternalAddon.cmake")
endif()

project(score_addon_simpleapi2 LANGUAGES CXX)

add_library(
  score_addon_simpleapi2
  Examples/Distortion.hpp
  Examples/CCC.hpp
  SimpleApi2/Concepts.hpp
  SimpleApi2/Executor.hpp
  SimpleApi2/ExecutorNode.hpp
  SimpleApi2/Layer.hpp
  SimpleApi2/Metadatas.hpp
  SimpleApi2/ProcessModel.hpp

  score_addon_simpleapi2.hpp
  score_addon_simpleapi2.cpp
)

target_link_libraries(score_addon_simpleapi2 PRIVATE score_plugin_engine)

setup_score_plugin(score_addon_simpleapi2)
