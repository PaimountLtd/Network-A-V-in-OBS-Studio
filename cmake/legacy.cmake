project(obs-ndi)

add_library(obs-ndi MODULE)
add_library(OBS::ndi ALIAS obs-ndi)

target_sources(
  obs-ndi
  PRIVATE src/plugin-main.cpp
          src/plugin-main.h
          src/obs-ndi-source.cpp
          src/obs-ndi-output.cpp
          src/obs-ndi-filter.cpp
          src/premultiplied-alpha-filter.cpp
          # src/main-output.cpp
          # src/preview-output.cpp
          # src/Config.cpp
          # src/forms/output-settings.cpp
          # src/main-output.h
          # src/preview-output.h
          # src/Config.h
          # src/forms/output-settings.h
          )

target_include_directories(obs-ndi PRIVATE lib/ndi)
target_link_libraries(obs-ndi PRIVATE OBS::libobs)

if(OS_WINDOWS)
  set(MODULE_DESCRIPTION "OBS NDI module")
  configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in obs-ndi.rc)

  target_sources(obs-ndi PRIVATE obs-ndi.rc)

endif()

set_target_properties(obs-ndi PROPERTIES FOLDER "plugins" PREFIX "")

setup_plugin_target(obs-ndi)
