#include "EditorUtilities.h"

#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/ProjectInfo.h"

#include <filesystem>

#ifdef _WIN32
static const std::unordered_set<std::string> g_setReservedSystemDirs
{
	"c:\\windows",
	"c:\\program files",
	"c:\\program files (x86)",
	"c:\\users\\default",
	"c:\\users\\public",
	"c:\\system32"
};
#else
static const std::unordered_set<std::string> g_setReservedSystemDirs
{
	"/proc",
	"/sys",
	"/dev",
	"/run",
	"/boot",
	"/lib",
	"/lib64"
};
#endif

namespace Feather {

	FileType GetFileType(const std::string& sPath)
	{
		std::filesystem::path path{ sPath };

		if (!std::filesystem::exists(path))
			return FileType::INVALID_TYPE;

		if (std::filesystem::is_directory(path))
			return FileType::FOLDER;

		std::string ext = path.extension().string();
		if (ext == ".wav" || ext == ".mp3" || ext == ".ogg")
			return FileType::SOUND;
		else if (ext == ".png" || ext == ".mp3" || ext == ".ogg")
			return FileType::IMAGE;
		else if (ext == ".lua" || ext == ".cpp" || ext == ".h" || ext == ".txt")
			return FileType::TXT;
		else
			return FileType::TXT;
	}

	std::vector<std::string> SplitStr(const std::string& str, char delimiter)
	{
		std::vector<std::string> tokens;
		size_t start{ 0 };
		size_t end{ str.find(delimiter) };

		while (end != std::string::npos)
		{
			tokens.push_back(str.substr(start, end - start));
			start = end + 1;
			end = str.find(delimiter, start);
		}
		tokens.push_back(str.substr(start));

		return tokens;
	}

	Texture* GetIconTexture(const std::string& sPath)
	{
		auto& assetManager = ASSET_MANAGER();
		switch (GetFileType(sPath))
		{
		case FileType::SOUND: return assetManager.GetTexture("music_icon").get();
		case FileType::IMAGE: return assetManager.GetTexture("image_icon").get();
		case FileType::TXT: return assetManager.GetTexture("file_icon").get();
		case FileType::FOLDER: return assetManager.GetTexture("folder_icon").get();
		default:
			return nullptr;
		}
	}

	bool IsReservedPathOrFile(const std::filesystem::path& path)
	{
		std::string lowerPath = path.string();
		std::ranges::transform(lowerPath, lowerPath.begin(), ::tolower);
		return std::ranges::any_of(g_setReservedSystemDirs, [&](const auto& dir) { return lowerPath.starts_with(dir); });
	}

	bool IsDefaultProjectPathOrFile(const std::filesystem::path& path, const ProjectInfo& projectInfo)
	{
		if (fs::is_directory(path))
		{
			bool isProjectPath = std::ranges::any_of(projectInfo.GetProjectPaths(), [&](const auto& pair) { return pair.second == path; });

			if (isProjectPath)
				return true;
		}

		if (fs::is_regular_file(path))
		{
			auto optMainLuaScript = projectInfo.GetMainLuaScriptPath();
			if (optMainLuaScript && *optMainLuaScript == path)
				return true;

			auto optScriptList = projectInfo.GetScriptListPath();
			if (optScriptList && *optScriptList == path)
				return true;
		}

		return false;
	}

}
