cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if(NOT TARGET score_lib_base)
  include("${SCORE_SOURCE_DIR}/cmake/ScoreExternalAddon.cmake")
endif()

project(score_addon_simpleapi2 LANGUAGES CXX)

add_library(
  score_addon_simpleapi2
  Examples/TrivialGenerator.hpp
  Examples/TrivialFilter.hpp

  Examples/SampleAccurateFilter.hpp
  Examples/SampleAccurateGenerator.hpp

  Examples/Empty.hpp

  Examples/AudioEffect.hpp
  Examples/AudioEffectWithSidechains.hpp

  Examples/RawPorts.hpp

  Examples/TextureGenerator.hpp

  Examples/Distortion.hpp
  Examples/CCC.hpp

  Examples/ControlGallery.hpp
  Examples/ZeroDependencyAudioEffect.hpp

  oscr/Attributes.hpp
  oscr/Concepts.hpp
  oscr/GpuNode.hpp
  oscr/Executor.hpp
  oscr/ExecutorNode.hpp
  oscr/Layer.hpp
  oscr/Metadatas.hpp
  oscr/ProcessModel.hpp

  score_addon_simpleapi2.hpp
  score_addon_simpleapi2.cpp
)

target_link_libraries(score_addon_simpleapi2 PRIVATE score_plugin_engine score_plugin_gfx)

setup_score_plugin(score_addon_simpleapi2)
