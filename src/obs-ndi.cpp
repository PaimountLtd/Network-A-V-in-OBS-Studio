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
#else
#include <dlfcn.h>
#endif

#include <sys/stat.h>

#include <obs-module.h>
#include <util/platform.h>

#include "obs-ndi.h"

#include <iostream>
#include <vector>
#include <sstream>

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("Stephane Lepin (Palakis)")
OBS_MODULE_USE_DEFAULT_LOCALE("obs-ndi", "en-US")

const NDIlib_v4* ndiLib = nullptr;

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
bool check_ndilib_version(std::string version);

#ifdef WIN32
HINSTANCE hGetProcIDDLL;
#endif

typedef const NDIlib_v4* (*NDIlib_v4_load_)(void);

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

    std::string version(ndiLib->NDIlib_version());
    
    if (!check_ndilib_version(version))
    {
        blog(LOG_ERROR, "Usupported NDI library version.");
        return false;
    }

    if (!ndiLib->NDIlib_initialize()) {
        blog(LOG_ERROR, "CPU unsupported by NDI library. Module won't load.");
        return false;
    }

    blog(LOG_INFO, "NDI library initialized successfully");

	if (!ndiLib->initialize()) {
		blog(LOG_ERROR, "CPU unsupported by NDI library. Module won't load.");
		return false;
	}

	blog(LOG_INFO, "NDI library initialized successfully (%s)", ndiLib->version());

	NDIlib_find_create_t find_desc = {0};
	find_desc.show_local_sources = true;
	find_desc.p_groups = NULL;
	ndi_finder = ndiLib->find_create_v2(&find_desc);

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

#ifdef WIN32
    if (hGetProcIDDLL)
        FreeLibrary(hGetProcIDDLL);
#else
//TODO
#endif
}

const char* obs_module_name() {
    return "obs-ndi";
}

const char* obs_module_description() {
    return "NDI input/output integration for OBS Studio";
}

#ifdef WIN32
const NDIlib_v3* load_ndilib() {
    const int szEnvVar = GetEnvironmentVariable(TEXT(NDILIB_REDIST_FOLDER), 0, 0);

    if (szEnvVar == 0) return nullptr;

    std::basic_string<TCHAR> strEnvVar;
    /* Reserve isn't good enough here since
     * using a basic_string as a buffer will
     * automatically adjust its size. */
    strEnvVar.resize(szEnvVar - 1);

		obs_frontend_add_event_callback([](enum obs_frontend_event event, void *private_data) {
			Config* conf = (Config*)private_data;

			if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
				if (conf->OutputEnabled) {
					main_output_start(conf->OutputName.toUtf8().constData());
				}
				if (conf->PreviewOutputEnabled) {
					preview_output_start(conf->PreviewOutputName.toUtf8().constData());
				}
			} else if (event == OBS_FRONTEND_EVENT_EXIT) {
				preview_output_stop();
				main_output_stop();

				preview_output_deinit();
				main_output_deinit();
			}
		}, (void*)conf);
	}

    NDIlib_v3_load_ lib_load = nullptr;
    // Load NewTek NDI Redist dll
    hGetProcIDDLL = LoadLibrary(strPath.data());

    if (hGetProcIDDLL == NULL) {
        blog(LOG_INFO, "ERROR: NDIlib_v3_load not found in loaded library");
    }
    else {
        blog(LOG_INFO, "NDI runtime loaded successfully");

	// Locate function in DLL.
	lib_load = (NDIlib_v4_load_)GetProcAddress(hGetProcIDDLL, "NDIlib_v4_load");

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

#else

const NDIlib_v3* load_ndilib() {
    std::vector<const char*> locations;
    const char* redist_folder = getenv("NDILIB_REDIST_FOLDER");

    if (redist_folder)
        locations.push_back(redist_folder);

    locations.push_back("/usr/lib/");
    locations.push_back("/usr/local/lib/");

    for (auto path: locations) {
        std::string lib = path;
        lib += NDILIB_LIBRARY_NAME;

        blog(LOG_INFO, "Trying to load lib at: %s", lib.c_str());

        FILE *file = fopen(lib.c_str(), "r");
        if (!file)
            continue;

        fclose(file);
        blog(LOG_INFO, "Found NDI library at '%s'",
            lib.c_str());

        void *handle = dlopen(lib.c_str(), RTLD_NOW);

        if (!handle)
            continue;

        blog(LOG_INFO, "NDI runtime loaded successfully");

        NDIlib_v4_load_ lib_load = (NDIlib_v4_load_)dlsym(handle, "NDIlib_v4_load");
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

#endif

bool check_ndilib_version(std::string version) {
    std::string versionNumber = version.substr(version.rfind(' ') + 1);

    std::string majorVersionNumber = versionNumber.substr(0, versionNumber.find('.'));
    versionNumber.erase(0, versionNumber.find('.') + 1);

    std::string minorVersionNumber = versionNumber.substr(0, versionNumber.find('.'));
    versionNumber.erase(0, versionNumber.find('.') + 1);
    try {
        if (std::stoi(majorVersionNumber) < NDI_LIB_MAJOR_VERSION_NUMBER)
        {
            return false;
        }

        if (std::stoi(majorVersionNumber) == NDI_LIB_MAJOR_VERSION_NUMBER &&
        std::stoi(minorVersionNumber) < NDI_LIB_MINOR_VERSION_NUMBER)
        {
            return false;
        }
    } catch (...) {
        if (version.find(" .1.0.0") != std::string::npos)  { // whitelist ndi broken version
            return true;
        }
        return false;
    }

    return true;
}
