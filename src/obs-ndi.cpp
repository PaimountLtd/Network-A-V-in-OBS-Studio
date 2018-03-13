/*
obs-ndi
Copyright (C) 2016-2018 Stéphane Lepin <steph  name of author

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; If not, see <https://www.gnu.org/licenses/>
*/

#ifdef _WIN32
#include <Windows.h>
#endif

#include <sys/stat.h>

#include <obs-module.h>
#include <util/platform.h>

#include "obs-ndi.h"
#include "main-output.h"
#include "preview-output.h"

#include <iostream>

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("Stephane Lepin (Palakis)")
OBS_MODULE_USE_DEFAULT_LOCALE("obs-ndi", "en-US")

const NDIlib_v3* ndiLib = nullptr;

extern struct obs_source_info create_ndi_source_info();
struct obs_source_info ndi_source_info;

extern struct obs_output_info create_ndi_output_info();
struct obs_output_info ndi_output_info;

extern struct obs_source_info create_ndi_filter_info();
struct obs_source_info ndi_filter_info;

extern struct obs_source_info create_ndi_audiofilter_info();
struct obs_source_info ndi_audiofilter_info;

extern struct obs_source_info create_alpha_filter_info();
struct obs_source_info alpha_filter_info;

const NDIlib_v3* load_ndilib();

HINSTANCE hGetProcIDDLL;

typedef const NDIlib_v3* (*NDIlib_v3_load_)(void);

NDIlib_find_instance_t ndi_finder;
obs_output_t* main_out;
bool main_output_running = false;

bool obs_module_load(void) {
    blog(LOG_INFO, "hello ! (version %s)", OBS_NDI_VERSION);
	
    ndiLib = load_ndilib();
    if (!ndiLib) {
		blog(LOG_ERROR, "Error when loading the library.");
        return false;
    }

    if (!ndiLib->NDIlib_initialize()) {
        blog(LOG_ERROR, "CPU unsupported by NDI library. Module won't load.");
        return false;
    }

    blog(LOG_INFO, "NDI library initialized successfully");

    NDIlib_find_create_t find_desc = {0};
    find_desc.show_local_sources = true;
    find_desc.p_groups = NULL;
    ndi_finder = ndiLib->NDIlib_find_create_v2(&find_desc);

    ndi_source_info = create_ndi_source_info();
    obs_register_source(&ndi_source_info);

    ndi_filter_info = create_ndi_filter_info();
    obs_register_source(&ndi_filter_info);

    ndi_audiofilter_info = create_ndi_audiofilter_info();
    obs_register_source(&ndi_audiofilter_info);

    alpha_filter_info = create_alpha_filter_info();
    obs_register_source(&alpha_filter_info);

    return true;
}

void obs_module_unload() {
    blog(LOG_INFO, "goodbye !");

    if (ndiLib) {
        ndiLib->NDIlib_find_destroy(ndi_finder);
        ndiLib->NDIlib_destroy();
    }

	if (hGetProcIDDLL)
		FreeLibrary(hGetProcIDDLL);
}

const char* obs_module_name() {
    return "obs-ndi";
}

const char* obs_module_description() {
    return "NDI input/output integration for OBS Studio";
}

const NDIlib_v3* load_ndilib() {
	std::string libFolder = getenv(NDILIB_REDIST_FOLDER);
	std::string libFile = libFolder + "\\" + NDILIB_LIBRARY_NAME;
	NDIlib_v3_load_ lib_load = nullptr;
	// Load NewTek NDI Redist dll
	hGetProcIDDLL = LoadLibraryA(libFile.c_str());

	if (hGetProcIDDLL == NULL) {
		blog(LOG_INFO, "ERROR: NDIlib_v3_load not found in loaded library");
	}
	else {
		blog(LOG_INFO, "NDI runtime loaded successfully");

		// Locate function in DLL.
		lib_load = (NDIlib_v3_load_)GetProcAddress(hGetProcIDDLL, "NDIlib_v3_load");

		// Check if function was located.
		if (!lib_load) {
			blog(LOG_INFO, "ERROR: NDIlib_v3_load not found in loaded library");
		}
		else {
			return lib_load();
						
		}
	}

    blog(LOG_ERROR, "Can't find the NDI library");
    return nullptr;
}
