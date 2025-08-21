#include "ProjectLoader.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/CoreUtils/ProjectInfo.h"
#include "Core/CoreUtils/Prefab.h"
#include "Renderer/Essentials/Shader.h"
#include "Renderer/Essentials/Texture.h"
#include "Renderer/Essentials/Font.h"
#include "Sounds/Essentials/Music.h"
#include "Sounds/Essentials/SoundFX.h"
#include "Filesystem/Serializers/JSONSerializer.h"
#include "Filesystem/Serializers/LuaSerializer.h"

#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Scene/SceneManager.h"
#include "Editor/Scene/SceneObject.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

static const std::map<Feather::EProjectFolderType, std::string> mapProjectDirs = {
		{ Feather::EProjectFolderType::Content,		"content" },
		{ Feather::EProjectFolderType::Scripts,		"content/scripts" },
		// Asset Folders
		{ Feather::EProjectFolderType::Assets,		"content/assets" },
		{ Feather::EProjectFolderType::SoundFx,		"content/assets/soundfx" },
		{ Feather::EProjectFolderType::Music,		"content/assets/music" },
		{ Feather::EProjectFolderType::Textures,	"content/assets/textures" },
		{ Feather::EProjectFolderType::Shaders,		"content/assets/shaders" },
		{ Feather::EProjectFolderType::Fonts,		"content/assets/fonts" },
		{ Feather::EProjectFolderType::Prefabs,		"content/assets/prefabs" },
		{ Feather::EProjectFolderType::Scenes,		"content/assets/scenes" },
		// Config Folders
		{ Feather::EProjectFolderType::Config,		"config"},
		{ Feather::EProjectFolderType::GameConfig,	"config/game"},
		{ Feather::EProjectFolderType::EditorConfig,"config/editor"},
};

namespace Feather {

	bool ProjectLoader::CreateNewProject(const std::string& projectName, const std::string& filepath)
	{
		auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
		F_ASSERT(projectInfo && "Project Info must exist");

		// Create the game filepath
		fs::path gameFilepath{ std::format("{}{}{}{}{}", filepath, PATH_SEPARATOR, projectName, PATH_SEPARATOR, "Feather") };

		if (fs::is_directory(gameFilepath))
		{
			F_ERROR("Project '{}' at '{}' already exists", projectName, filepath);
			return false;
		}

		std::error_code ec;
		for (const auto& [eFolderType, sSubDir] : mapProjectDirs)
		{
			fs::path fullpath{ gameFilepath / fs::path{ sSubDir } };

			if (!fs::create_directories(fullpath, ec))
			{
				F_ERROR("Failed to create directories: '{}' - {}", fullpath.string(), ec.message());
				return false;
			}

			projectInfo->AddFolderPath(eFolderType, fullpath);
		}

		projectInfo->SetProjectName(projectName);
		projectInfo->SetProjectPath(gameFilepath);

		return CreateProjectFile(projectName, gameFilepath.string());
	}

	bool ProjectLoader::LoadProject(const std::string& filepath)
	{
		std::ifstream procFile;
		procFile.open(filepath);

		if (!procFile.is_open())
		{
			F_ERROR("Failed to open project file '{}'", filepath);
			return false;
		}

		std::stringstream ss;
		ss << procFile.rdbuf();
		std::string contents = ss.str();
		rapidjson::StringStream jsonStr{ contents.c_str() };

		rapidjson::Document doc;
		doc.ParseStream(jsonStr);

		if (doc.HasParseError() || !doc.IsObject())
		{
			F_ERROR("Failed to load Project: File: '{}' is not valid JSON: {} - {}",
				filepath,
				rapidjson::GetParseError_En(doc.GetParseError()),
				doc.GetErrorOffset());
			return false;
		}

		// Get the project data
		if (!doc.HasMember("project_data"))
		{
			F_ERROR("Failed to load project: File '{}' - Expecting \"project_data\" member in project file", filepath);
			return false;
		}

		auto& mainRegistry = MAIN_REGISTRY();
		auto& projectInfo = mainRegistry.GetContext<ProjectInfoPtr>();
		F_ASSERT(projectInfo && "Project Info must be valid!");

		// We need the project filepath saved
		projectInfo->SetProjectFilePath(filepath);

		const rapidjson::Value& projectData = doc["project_data"];

		// Set the project name. The actual project name might be different to the project files name
		projectInfo->SetProjectName(projectData["project_name"].GetString());

		// We need to load all the assets
		if (!projectData.HasMember("assets"))
		{
			F_ERROR("Failed to load project: File '{}' - Expecting \"assets\" member in project file", filepath);
			return false;
		}

		// Get Content Path
		std::filesystem::path filePath{ filepath };
		fs::path projectPath{ filePath.parent_path() };
		projectInfo->SetProjectPath(projectPath);

		// Setup Project Folders
		std::error_code ec;
		for (const auto& [eFolderType, subDir] : mapProjectDirs)
		{
			fs::path fullPath{ projectPath / subDir };
			if (!fs::exists(fullPath, ec))
			{
				F_ERROR("Failed to load project: Failed to setup project folders. {}", ec.message());
				return false;
			}

			if (!projectInfo->AddFolderPath(eFolderType, fullPath))
			{
				F_ERROR("Failed to setup project folder '{}'", fullPath.string());
				return false;
			}
		}

		// Setup Project Files
		if (auto optScriptPath = projectInfo->TryGetFolderPath(EProjectFolderType::Scripts))
		{
			fs::path mainScriptPath = *optScriptPath / "main.lua";
			if (!fs::exists(mainScriptPath))
			{
				F_ERROR("Failed to load project: main.lua file was not found at '{}'", mainScriptPath.string());
				return false;
			}
		}

		// Setup Project Files
		if (auto optGameConfigPath = projectInfo->TryGetFolderPath(EProjectFolderType::GameConfig))
		{
			fs::path scriptListPath = *optGameConfigPath / "script_list.lua";
			if (!fs::exists(scriptListPath))
			{
				F_ERROR("Failed to load project: script_list.lua file was not found at '{}'", scriptListPath.string());
				return false;
			}
		}

		// Get the project path before we adjust it to the content path
		CORE_GLOBALS().SetProjectPath(projectPath.string());

		auto optContentFolderPath = projectInfo->TryGetFolderPath(EProjectFolderType::Content);
		F_ASSERT(optContentFolderPath && "Content folder not set correctly in project info");

		// Check to see if there is a main lua path
		if (projectData.HasMember("main_lua_script"))
		{
			auto mainLuaScript = *optContentFolderPath / projectData["main_lua_script"].GetString();
			if (!fs::exists(mainLuaScript))
			{
				F_ERROR("Failed to set main lua script path: '{}' does not exist", mainLuaScript.string());
				return false;
			}

			projectInfo->SetMainLuaScriptPath(mainLuaScript);
		}

		// Check to see if there is a file icon and load it
		if (projectData.HasMember("file_icon"))
		{
			std::string sFileIcon = projectData["file_icon"].GetString();
			if (!sFileIcon.empty())
			{
				auto fileIconPath = *optContentFolderPath / sFileIcon;
				if (!fs::exists(fileIconPath))
				{
					F_ERROR("Failed to set game file icon path: '{}' does not exist", fileIconPath.string());
					return false;
				}

				projectInfo->SetFileIconPath(fileIconPath);
			}
		}

		auto optGameConfigPath = projectInfo->TryGetFolderPath(EProjectFolderType::GameConfig);
		F_ASSERT(optGameConfigPath && "Game Config folder path has not been setup correctly in project info");

		fs::path scriptListPath = *optGameConfigPath / "script_list.lua";
		if (!fs::exists(scriptListPath))
		{
			F_ERROR("Failed to load project. ScriptList was not found at path [{}]", scriptListPath.string());
			return false;
		}

		projectInfo->SetScriptListPath(scriptListPath);

		auto& coreGlobals = CORE_GLOBALS();

		if (projectData.HasMember("game_type"))
		{
			coreGlobals.SetGameType(coreGlobals.GetGameTypeFromStr(projectData["game_type"].GetString()));
		}

		if (projectData.HasMember("copyright"))
		{
			std::string sCopyRight = projectData["copyright"].GetString();
			projectInfo->SetCopyRightNotice(sCopyRight);
		}

		if (projectData.HasMember("version"))
		{
			std::string sVersion = projectData["version"].GetString();
			projectInfo->SetProjectVersion(sVersion);
		}

		if (projectData.HasMember("description"))
		{
			std::string sDescription = projectData["description"].GetString();
			projectInfo->SetProjectDescription(sDescription);
		}

		const rapidjson::Value& assets = projectData["assets"];
		auto& assetManager = ASSET_MANAGER();

		// Load all textures into the asset manager
		if (assets.HasMember("textures"))
		{
			const rapidjson::Value& textures = assets["textures"];

			if (!textures.IsArray())
			{
				F_ERROR("Failed to load project: File '{}' - Expecting \"textures \" must be an array", filepath);
				return false;
			}

			for (const auto& jsonTexture : textures.GetArray())
			{
				// Assets path's should be saved as follows "assets/[asset_type]/[extra_folders opt]/file"
				std::string textureName{ jsonTexture["name"].GetString() };
				std::string jsonTexturePath = jsonTexture["path"].GetString();
				fs::path texturePath = *optContentFolderPath / jsonTexturePath;

				if (!assetManager.AddTexture(textureName, texturePath.string(), jsonTexture["isPixelArt"].GetBool(), jsonTexture["isTileset"].GetBool()))
				{
					F_ERROR("Failed to load texture '{}' at path '{}'", textureName, texturePath.string());
					// Should we stop loading or finish?
				}
			}
		}

		// Load all soundfx to the asset manager
		if (assets.HasMember("soundfx"))
		{
			const rapidjson::Value& soundfx = assets["soundfx"];

			if (!soundfx.IsArray())
			{
				F_ERROR("Failed to load project: File '{}' - Expecting \"soundfx\" must be an array", filepath);
				return false;
			}

			for (const auto& jsonSoundFx : soundfx.GetArray())
			{
				std::string soundFxName{ jsonSoundFx["name"].GetString() };
				std::string jsonSoundFxPath = jsonSoundFx["path"].GetString();
				fs::path soundFxPath = *optContentFolderPath / jsonSoundFxPath;

				if (!assetManager.AddSoundFx(soundFxName, soundFxPath.string()))
				{
					F_ERROR("Failed to load soundfx '{}' at path '{}'", soundFxName, soundFxPath.string());
					// Should we stop loading or finish?
				}
			}
		}

		// Load all music to the asset manager
		if (assets.HasMember("music"))
		{
			const rapidjson::Value& music = assets["music"];

			if (!music.IsArray())
			{
				F_ERROR("Failed to load project: File '{}' - Expecting \"music\" must be an array", filepath);
				return false;
			}

			for (const auto& jsonMusic : music.GetArray())
			{
				std::string musicName{ jsonMusic["name"].GetString() };
				std::string jsonMusicPath = jsonMusic["path"].GetString();
				fs::path musicPath = *optContentFolderPath / jsonMusicPath;

				if (!assetManager.AddMusic(musicName, musicPath.string()))
				{
					F_ERROR("Failed to load music '{}' at path '{}'", musicName, musicPath.string());
					// Should we stop loading or finish?
				}
			}
		}

		// Load all fonts to the asset manager
		if (assets.HasMember("fonts"))
		{
			const rapidjson::Value& fonts = assets["fonts"];

			if (!fonts.IsArray())
			{
				F_ERROR("Failed to load project: File '{}' - Expecting \"fonts\" must be an array", filepath);
				return false;
			}

			for (const auto& jsonFonts : fonts.GetArray())
			{
				std::string fontName{ jsonFonts["name"].GetString() };
				std::string jsonFontPath = jsonFonts["path"].GetString();
				fs::path fontPath = *optContentFolderPath / jsonFontPath;

				if (!assetManager.AddFont(fontName, fontPath.string(), jsonFonts["fontSize"].GetFloat()))
				{
					F_ERROR("Failed to load fonts '{}' at path '{}'", fontName, fontPath.string());
					// Should we stop loading or finish?
				}
			}
		}

		// Load all scenes to the scene manager
		if (assets.HasMember("scenes"))
		{
			const rapidjson::Value& scenes = assets["scenes"];

			if (!scenes.IsArray())
			{
				F_ERROR("Failed to load project: File '{}' - Expecting \"scenes\" must be an array", filepath);
				return false;
			}

			auto& sceneManager = SCENE_MANAGER();
			for (const auto& jsonScenes : scenes.GetArray())
			{
				std::string sceneName{ jsonScenes["name"].GetString() };
				std::string jsonScenePath = jsonScenes["sceneData"].GetString();
				fs::path scenePath = *optContentFolderPath / jsonScenePath;

				if (!sceneManager.AddSceneObject(sceneName, scenePath.string()))
				{
					F_ERROR("Failed to load scene '{}' at path '{}'", sceneName, scenePath.string());
				}
			}
		}

		// Load all prefabs to the scene manager
		if (assets.HasMember("prefabs"))
		{
			const rapidjson::Value& prefabs = assets["prefabs"];

			if (!prefabs.IsArray())
			{
				F_ERROR("Failed to load project file '{}': Expecting \"prefabs\" must be an array", filepath);
				return false;
			}

			for (const auto& jsonPrefab : prefabs.GetArray())
			{
				std::string sName{ jsonPrefab["name"].GetString() };
				std::string jsonPrefabPath = jsonPrefab["path"].GetString();
				fs::path prefabPath = *optContentFolderPath / jsonPrefabPath;

				if (auto prefab = PrefabCreator::CreatePrefab(prefabPath.string()))
				{
					if (!assetManager.AddPrefab(sName, std::move(prefab)))
					{
						F_ERROR("Failed to load scene: {}", sName);
					}
				}
				else
				{
					F_ERROR("Failed to load prefab '{}' from path '{}'", sName, prefabPath.string());
				}
			}
		}

		if (projectData.HasMember("physics"))
		{
			const rapidjson::Value& physics = projectData["physics"];
			bool bEnabled = physics["enabled"].GetBool();
			if (bEnabled)
			{
				coreGlobals.EnablePhysics();
			}
			else
			{
				coreGlobals.DisablePhysics();
			}

			coreGlobals.SetGravity(physics["gravityScale"].GetFloat());
			coreGlobals.SetVelocityIterations(physics["velocityIterations"].GetInt());
			coreGlobals.SetPositionIterations(physics["positionIterations"].GetInt());
		}

		return true;
	}

	bool ProjectLoader::SaveLoadedProject(const ProjectInfo& projectInfo)
	{
		auto optProjectFilePath = projectInfo.GetProjectFilePath();
		F_ASSERT(optProjectFilePath && "Project file path not set correctly");

		if (!fs::exists(*optProjectFilePath))
		{
			F_ERROR("Failed to save project file for '{}' at path '{}'", projectInfo.GetProjectName(), optProjectFilePath->string());
			return false;
		}

		std::unique_ptr<JSONSerializer> serializer{ nullptr };

		try
		{
			serializer = std::make_unique<JSONSerializer>(optProjectFilePath->string());
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to save tilemap '{}' - '{}'", optProjectFilePath->string(), ex.what());
			return false;
		}

		auto& assetManager = ASSET_MANAGER();
		auto& sceneMananger = SCENE_MANAGER();

		if (!sceneMananger.SaveAllScenes())
			F_ERROR("Failed to save all scenes");

		auto& coreGlobals = CORE_GLOBALS();
		serializer->StartDocument();
		serializer->StartNewObject("warnings");
		serializer->AddKeyValuePair("warning", std::string{ "This file is engine generated" })
			.AddKeyValuePair("warning", std::string{ "DO NOT CHANGE unless you know what you are doing" });
		serializer->EndObject(); // Warnings

		auto optMainLuaScript = projectInfo.GetMainLuaScriptPath();
		F_ASSERT(optMainLuaScript && "Main Lua script not setup correctly in project info");
		std::string mainLuaScript = optMainLuaScript->string();

		std::string fileIconPath{};
		auto optGameFileIcon = projectInfo.GetFileIconPath();
		if (optGameFileIcon)
		{
			std::string gameFileIcon{ optGameFileIcon->string() };
			fileIconPath = gameFileIcon.substr(gameFileIcon.find(CONTENT_FOLDER) + CONTENT_FOLDER.size() + 1);
		}

		serializer->StartNewObject("project_data")
			.AddKeyValuePair("project_name", projectInfo.GetProjectName())
			.AddKeyValuePair("main_lua_script", mainLuaScript.substr(mainLuaScript.find(SCRIPTS)))
			.AddKeyValuePair("file_icon", fileIconPath)
			.AddKeyValuePair("game_type", coreGlobals.GetGameTypeStr(coreGlobals.GetGameType()))
			.AddKeyValuePair("copyright", projectInfo.GetCopyRightNotice())
			.AddKeyValuePair("version", projectInfo.GetProjectVersion())
			.AddKeyValuePair("description", projectInfo.GetProjectDescription())
			.StartNewObject("assets");

		serializer->StartNewArray("textures");
		for (const auto& [name, texture] : assetManager.GetAllTextures())
		{
			if (!texture || texture->IsEditorTexture())
				continue;

			// Get Relative to assets path
			std::string texturePath = texture->GetPath().substr(texture->GetPath().find(ASSETS));
			F_ASSERT(!texturePath.empty());
			serializer->StartNewObject()
				.AddKeyValuePair("name", name)
				.AddKeyValuePair("path", texturePath)
				.AddKeyValuePair("isPixelArt", texture->GetType() == Texture::TextureType::PIXEL)
				.AddKeyValuePair("isTileset", texture->IsTileset())
				.EndObject();

		}
		serializer->EndArray(); // Textures

		serializer->StartNewArray("soundfx");
		for (const auto& [name, sound] : assetManager.GetAllSoundFx())
		{
			std::string soundFxPath = sound->GetFilename().substr(sound->GetFilename().find(ASSETS));
			serializer->StartNewObject()
				.AddKeyValuePair("name", name)
				.AddKeyValuePair("path", soundFxPath)
				.EndObject();
		}
		serializer->EndArray(); // SoundFx

		serializer->StartNewArray("music");
		for (const auto& [name, music] : assetManager.GetAllMusic())
		{
			std::string musicPath = music->GetFilename().substr(music->GetFilename().find(ASSETS));
			serializer->StartNewObject()
				.AddKeyValuePair("name", name)
				.AddKeyValuePair("path", musicPath)
				.EndObject();
		}
		serializer->EndArray(); // Music

		serializer->StartNewArray("scenes");

		for (const auto& [name, scene] : sceneMananger.GetAllScenes())
		{
			std::string scenePath = scene->GetSceneDataPath().substr(scene->GetSceneDataPath().find(ASSETS));
			serializer->StartNewObject()
				.AddKeyValuePair("name", name)
				.AddKeyValuePair("sceneData", scenePath)
				.EndObject();
		}
		serializer->EndArray(); // Scenes

		serializer->StartNewArray("prefabs");

		for (const auto& [sName, pPrefab] : assetManager.GetAllPrefabs())
		{
			std::string sFilepath = pPrefab->GetFilepath().substr(pPrefab->GetFilepath().find(ASSETS));
			serializer->StartNewObject().AddKeyValuePair("name", sName).AddKeyValuePair("path", sFilepath).EndObject();
		}

		serializer->EndArray(); // Prefabs

		serializer->EndObject(); // Assets
		serializer->StartNewObject("physics")
			.AddKeyValuePair("enabled", coreGlobals.IsPhysicsEnabled())
			.AddKeyValuePair("gravityScale", coreGlobals.GetGravity())
			.AddKeyValuePair("velocityIterations", coreGlobals.GetVelocityIterations())
			.AddKeyValuePair("positionIterations", coreGlobals.GetPositionIterations())
			.EndObject();		 // Physics
		serializer->EndObject(); // Project Data

		return serializer->EndDocument();
	}

	bool ProjectLoader::CreateProjectFile(const std::string& projectName, const std::string& filepath)
	{
		if (!std::filesystem::is_directory(filepath))
		{
			F_ERROR("Failed to create project file for '{}' at path '{}'", projectName, filepath);
			return false;
		}

		if (!CreateMainLuaScript(projectName, filepath))
		{
			F_ERROR("Failed to create main lua script");
			return false;
		}

		if (!CreateScriptListFile())
		{
			F_ERROR("Failed to create Script List");
			false;
		}

		std::string projectFile{ filepath + PATH_SEPARATOR + projectName + std::string{ FEATHER_PRJ_FILE_EXT } };
		std::unique_ptr<JSONSerializer> serializer{ nullptr };

		try
		{
			serializer = std::make_unique<JSONSerializer>(projectFile);
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to save tilemap '{}': '{}'", projectFile, ex.what());
			return false;
		}

		// We want to grab the project file path
		auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
		projectInfo->SetProjectFilePath(fs::path{ projectFile });

		auto optMainLuaScript = projectInfo->GetMainLuaScriptPath();
		F_ASSERT(optMainLuaScript && "Main lua script not setup in project info");
		std::string mainLuaScript = optMainLuaScript->string();

		serializer->StartDocument();
		serializer->StartNewObject("warnings");
		serializer->AddKeyValuePair("warning", std::string{ "This file is engine generated" })
			.AddKeyValuePair("warning", std::string{ "DO NOT CHANGE unless you know what you are doing" });
		serializer->EndObject(); // Warnings

		serializer->StartNewObject("project_data")
			.AddKeyValuePair("project_name", projectName)
			.AddKeyValuePair("main_lua_file", mainLuaScript.substr(mainLuaScript.find(SCRIPTS)))
			.AddKeyValuePair("game_type", std::string{ "No Type" })
			.StartNewObject("assets")
			.StartNewArray("textures")
			.EndArray() // Textures
			.StartNewArray("soundfx")
			.EndArray() // SoundFx
			.StartNewArray("music")
			.EndArray() // Music
			.StartNewArray("scenes")
			.EndArray()	 // Scenes
			.EndObject() // Assets
			.StartNewObject("physics")
			.AddKeyValuePair("enabled", true)
			.AddKeyValuePair("gravityScale", 9.8f)
			.AddKeyValuePair("velocityIterations", 8)
			.AddKeyValuePair("positionIterations", 10)
			.EndObject();		  // Physics
		serializer->EndObject(); // Project Data

		return serializer->EndDocument();
	}

	bool ProjectLoader::CreateMainLuaScript(const std::string& projectName, const std::string& filepath)
	{
		fs::path mainLuaFilePath{ filepath };
		mainLuaFilePath /= "content";
		mainLuaFilePath /= "scripts";
		mainLuaFilePath /= "main.lua";

		auto luaSerializer = std::make_unique<LuaSerializer>(mainLuaFilePath.string());
		F_ASSERT(luaSerializer);

		// Save the main lua file path
		MAIN_REGISTRY().GetContext<ProjectInfoPtr>()->SetMainLuaScriptPath(mainLuaFilePath);

		luaSerializer->AddBlockComment("\tMain Lua script. This is needed to run all scripts in the editor."
									   "\n\tGenerated by the engine on project creation."
									   "\n\tDON'T CHANGE unless you know what you are doing!");

		luaSerializer->AddComment("The engine looks for these two functions.")
			.AddComment("Please add your code inside of the update and render functions as needed.");

		luaSerializer->StartNewTable("main")
			.StartNewTable("1", true, true)
			.AddKeyValuePair("update", "function() end", true, true)
			.EndTable()
			.StartNewTable("2", true, true)
			.AddKeyValuePair("render", "function() end", true, true)
			.EndTable()
			.EndTable();

		return luaSerializer->FinishStream();
	}

	bool ProjectLoader::CreateScriptListFile()
	{
		auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();

		auto optPath = projectInfo->TryGetFolderPath(EProjectFolderType::GameConfig);
		if (!optPath)
		{
			F_ERROR("Failed to create script list file. Game Config path does not exist");
			return false;
		}

		fs::path scriptListPath = *optPath / "script_list.lua";

		if (!fs::exists(scriptListPath))
		{
			std::ofstream file{ scriptListPath.string() };
			file.close();
		}

		std::error_code ec;
		if (!fs::exists(scriptListPath, ec))
		{
			F_ERROR("Failed to create script list: {}", ec.message());
			return false;
		}

		projectInfo->SetScriptListPath(scriptListPath);

		return true;
	}

}
