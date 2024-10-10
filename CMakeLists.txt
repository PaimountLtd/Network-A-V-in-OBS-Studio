cmake_minimum_required(VERSION 3.16...3.26)

legacy_check()

add_library(obs-ndi MODULE)
add_library(OBS::ndi ALIAS obs-ndi)

target_sources(
  obs-ndi
  PRIVATE src/plugin-main.cpp
          src/plugin-main.h
          src/obs-ndi-source.cpp
          src/obs-ndi-output.cpp
          src/obs-ndi-filter.cpp
          src/premultiplied-alpha-filter.cpp)

target_include_directories(obs-ndi PRIVATE lib/ndi)
target_link_libraries(obs-ndi PRIVATE OBS::libobs)

target_include_directories(obs-ndi PRIVATE ${CMAKE_SOURCE_DIR}/lib/ndi)

set_target_properties_obs(obs-ndi PROPERTIES FOLDER plugins PREFIX "")