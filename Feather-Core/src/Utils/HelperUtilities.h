#pragma once
#include <string>

#define BASE_PATH          \
	std::string            \
	{                      \
		SDL_GetBasePath()  \
	}

#ifdef _WIN32
constexpr char PATH_SEPARATOR = '\\';
#define DEFAULT_PROJECT_PATH BASE_PATH + "FeatherProjects"
#else
constexpr char PATH_SEPARATOR = '/';
#define DEFAULT_PROJECT_PATH BASE_PATH + PATH_SEPARATOR + "FeatherProjects"
#endif

#define SCRIPTS "scripts"
#define ASSETS "assets"

constexpr const char* FEATHER_PRJ_FILE_EXT = ".fprj";

namespace Feather {

	struct SpriteLayerParams
	{
		std::string layerName{ "NewLayer" };
		bool isVisible{ true };
	};

}
