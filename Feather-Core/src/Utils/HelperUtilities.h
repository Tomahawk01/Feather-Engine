#pragma once

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
#define CONTENT "content"

constexpr const std::string_view CONTENT_FOLDER = "content";
constexpr const std::string_view FEATHER_PRJ_FILE_EXT = ".fprj";

namespace Feather {

	struct SpriteLayerParams
	{
		std::string layerName{ "NewLayer" };
		bool isVisible{ true };
		int layer{ -1 };

		friend bool operator==(const SpriteLayerParams& a, const SpriteLayerParams& b)
		{
			return a.layerName == b.layerName && a.isVisible == b.isVisible && a.layer == b.layer;
		}
	};

}
