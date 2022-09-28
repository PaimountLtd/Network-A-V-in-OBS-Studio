cmake_minimum_required(VERSION 3.5)
project(obs-ndi VERSION 4.6.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)


if (TARGET libobs) 
	set(NDI_EXTERNAL FALSE)
	set(NDI_PLUGIN_DIRECTORY "${OBS_PLUGIN_DESTINATION}")
	set(NDI_DATA_DIRECTORY   "${OBS_DATA_DESTINATION}/obs-plugins/obs-ndi")
else ()
	set(NDI_EXTERNAL TRUE)
	include(external/FindLibObs.cmake)
	find_package(LibObs REQUIRED)
endif ()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH 64)
else()
	set(ARCH 32)
endif()

set(obs-ndi_SOURCES
	src/obs-ndi.cpp
	src/obs-ndi-source.cpp
	src/obs-ndi-output.cpp
	src/obs-ndi-filter.cpp
	src/premultiplied-alpha-filter.cpp)

set(obs-ndi_HEADERS src/obs-ndi.h)

add_library(obs-ndi MODULE
	${obs-ndi_SOURCES}
	${obs-ndi_HEADERS})

include_directories("lib/ndi")

target_link_libraries(obs-ndi libobs)

set_target_properties(obs-ndi PROPERTIES PREFIX "")

setup_plugin_target(obs-ndi)

