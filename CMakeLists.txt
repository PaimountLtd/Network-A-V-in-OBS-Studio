cmake_minimum_required(VERSION 3.5)
project(obs-ndi VERSION 4.9.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") 
    set(CMAKE_CXX_FLAGS "-mfpu=neon")
endif()

if (TARGET libobs) 
	set(NDI_EXTERNAL FALSE)
	set(NDI_PLUGIN_DIRECTORY "${OBS_PLUGIN_DESTINATION}")
	set(NDI_DATA_DIRECTORY   "${OBS_DATA_DESTINATION}/obs-plugins/obs-ndi")
else ()
	set(NDI_EXTERNAL TRUE)
	if (WIN32 OR APPLE)
		include(external/FindLibObs.cmake)
	endif()
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

# Windows
if(WIN32)
	if (NDI_EXTERNAL)
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

    if(MSVC)
        # Enable Multicore Builds and disable FH4 (to not depend on VCRUNTIME140_1.DLL)
	    add_definitions(/MP /d2FH4-)
    endif()

	add_definitions(/O2)
	string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	message("${CMAKE_CXX_FLAGS_RELEASE}")

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

target_link_libraries(obs-ndi libobs)

# OSX
if(APPLE)
	set_target_properties(obs-ndi PROPERTIES PREFIX "")

	install_obs_plugin_with_data(obs-ndi data)
endif()

# Linux
if(UNIX AND NOT APPLE)
	include(GNUInstallDirs)

	if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") 
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -mtune=arm7 -Ofast")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -mtune=core2 -Ofast")
	endif()

	set_target_properties(obs-ndi PROPERTIES PREFIX "")
	target_link_libraries(obs-ndi obs-frontend-api)

	file(GLOB locale_files data/locale/*.ini)

	install(TARGETS obs-ndi LIBRARY
		DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/obs-plugins")
	if(${USE_UBUNTU_FIX})
		install(TARGETS obs-ndi LIBRARY
			DESTINATION "/usr/lib/obs-plugins")
	endif()

	install(FILES ${locale_files}
		DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/obs/obs-plugins/obs-ndi/locale")
endif()
