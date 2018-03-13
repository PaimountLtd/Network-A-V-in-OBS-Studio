cmake_minimum_required(VERSION 3.5)
project(obs-ndi)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)


if (TARGET libobs) 
	set(NDI_PLUGIN_DIRECTORY "${OBS_PLUGIN_DESTINATION}")
	set(NDI_DATA_DIRECTORY   "${OBS_DATA_DESTINATION}/obs-plugins/obs-ndi")
else ()
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

include_directories("lib/ndi/Include")
target_link_libraries(obs-ndi libobs)

# Windows
if(WIN32)
	if (NOT TARGET libobs)
		if(ARCH EQUAL 64)
			set(OBS_ARCH_NAME "64bit")
			set(OBS_BUILDDIR_ARCH "build64")
		else()
			set(OBS_ARCH_NAME "32bit")
			set(OBS_BUILDDIR_ARCH "build32")
			add_definitions(/arch:SSE2)
		endif()
		set(NDI_PLUGIN_DIRECTORY "${CMAKE_INSTALL_PREFIX}/obs-plugins/${OBS_ARCH_NAME}")
		set(NDI_DATA_DIRECTORY "${CMAKE_INSTALL_PREFIX}/data/obs-plugins/obs-ndi")
	endif ()

	add_definitions(/O2)
	string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	message("${CMAKE_CXX_FLAGS_RELEASE}")

	find_package(w32-pthreads REQUIRED)
	target_link_libraries(obs-ndi w32-pthreads)

	install(
		FILES "$<TARGET_FILE:obs-ndi>"
		DESTINATION ${NDI_PLUGIN_DIRECTORY}
		CONFIGURATIONS Release RelWithDebInfo
	)

	install(
		FILES "$<TARGET_PDB_FILE:obs-ndi>"
		DESTINATION ${NDI_PLUGIN_DIRECTORY}
		CONFIGURATIONS Release RelWithDebInfo
		OPTIONAL
	)

	install(
		DIRECTORY data/
		DESTINATION ${NDI_DATA_DIRECTORY}
	)
endif()

# OSX
if(APPLE)
	set_target_properties(obs-ndi PROPERTIES PREFIX "")
endif()

# Linux
if(UNIX AND NOT APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -mtune=core2 -Ofast")

	if(ARCH EQUAL 64)
		set(ARCH_NAME "x86_64")
	else()
		set(ARCH_NAME "i686")
	endif()

	set_target_properties(obs-ndi PROPERTIES PREFIX "")

	install(TARGETS obs-ndi
		LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/obs-plugins)
	install(FILES data/locale/en-US.ini data/locale/fr-FR.ini
		DESTINATION "${CMAKE_INSTALL_PREFIX}/share/obs/obs-plugins/obs-ndi/locale")
endif()
