#include "ProjectLoader.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/CoreUtils/SaveProject.h"
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

#include <filesystem>

namespace Feather {

	bool ProjectLoader::CreateNewProject(const std::string& projectName, const std::string& filepath)
	{
		auto& saveProject = MAIN_REGISTRY().GetContext<std::shared_ptr<SaveProject>>();
		F_ASSERT(saveProject && "Save project must exist");

		// Create the game filepath
		std::string gameFilepath = std::format("{}{}{}{}{}", filepath, PATH_SEPARATOR, projectName, PATH_SEPARATOR, "Feather");

		if (std::filesystem::is_directory(gameFilepath))
		{
			F_ERROR("Project '{}' at '{}' already exists", projectName, filepath);
			return false;
		}

		char sep{ PATH_SEPARATOR };
		gameFilepath += sep;
		std::error_code ec;
		if (!std::filesystem::create_directories(gameFilepath + "content", ec) ||
			!std::filesystem::create_directories(gameFilepath + "content" + sep + "scripts", ec) ||
			!std::filesystem::create_directories(gameFilepath + "content" + sep + "assets", ec) ||
			!std::filesystem::create_directories(gameFilepath + "content" + sep + "assets" + sep + "soundfx", ec) ||
			!std::filesystem::create_directories(gameFilepath + "content" + sep + "assets" + sep + "music", ec) ||
			!std::filesystem::create_directories(gameFilepath + "content" + sep + "assets" + sep + "textures", ec) ||
			!std::filesystem::create_directories(gameFilepath + "content" + sep + "assets" + sep + "shaders", ec) ||
			!std::filesystem::create_directories(gameFilepath + "content" + sep + "assets" + sep + "fonts", ec) ||
			!std::filesystem::create_directories(gameFilepath + "content" + sep + "assets" + sep + "prefabs", ec) ||
			!std::filesystem::create_directories(gameFilepath + "content" + sep + "assets" + sep + "scenes", ec))
		{
			F_ERROR("Failed to create directories - {}", ec.message());
			// TODO: Delete any created directories?
			return false;
		}

		saveProject->projectName = projectName;
		saveProject->projectPath = gameFilepath;

		return CreateProjectFile(saveProject->projectName, saveProject->projectPath);
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
		auto& saveProject = mainRegistry.GetContext<std::shared_ptr<SaveProject>>();

		F_ASSERT(saveProject && "Save Project must be valid!");

		// We need the project filepath saved
		saveProject->projectFilePath = filepath;

		const rapidjson::Value& projectData = doc["project_data"];

		// Set the project name. The actual project name might be different to the project files name
		saveProject->projectName = projectData["project_name"].GetString();

		// We need to load all the assets
		if (!projectData.HasMember("assets"))
		{
			F_ERROR("Failed to load project: File '{}' - Expecting \"assets\" member in project file", filepath);
			return false;
		}

		// Get Content Path
		std::filesystem::path filePath{ filepath };
		std::string contentPath = filePath.parent_path().string();

		// Get the project path before we adjust it to the content path
		saveProject->projectPath = contentPath + PATH_SEPARATOR;
		CORE_GLOBALS().SetProjectPath(saveProject->projectPath);

		contentPath += PATH_SEPARATOR;
		contentPath += "content";
		contentPath += PATH_SEPARATOR;

		// Check to see if there is a main lua path
		if (projectData.HasMember("main_lua_script"))
			saveProject->mainLuaScript = contentPath + projectData["main_lua_script"].GetString();

		auto& coreGlobals = CORE_GLOBALS();

		if (projectData.HasMember("game_type"))
		{
			coreGlobals.SetGameType(coreGlobals.GetGameTypeFromStr(projectData["game_type"].GetString()));
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
				std::string texturePath{ contentPath + jsonTexture["path"].GetString() };

				if (!assetManager.AddTexture(textureName, texturePath, jsonTexture["isPixelArt"].GetBool(), jsonTexture["isTileset"].GetBool()))
				{
					F_ERROR("Failed to load texture '{}' at path '{}'", textureName, texturePath);
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
				std::string soundFxPath{ contentPath + jsonSoundFx["path"].GetString() };

				if (!assetManager.AddSoundFx(soundFxName, soundFxPath))
				{
					F_ERROR("Failed to load soundfx '{}' at path '{}'", soundFxName, soundFxPath);
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
				std::string musicPath{ contentPath + jsonMusic["path"].GetString() };

				if (!assetManager.AddMusic(musicName, musicPath))
				{
					F_ERROR("Failed to load music '{}' at path '{}'", musicName, musicPath);
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
				std::string fontPath{ contentPath + jsonFonts["path"].GetString() };

				if (!assetManager.AddFont(fontName, fontPath, jsonFonts["fontSize"].GetFloat()))
				{
					F_ERROR("Failed to load fonts '{}' at path '{}'", fontName, fontPath);
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
				std::string sceneDataPath{ contentPath + jsonScenes["sceneData"].GetString() };

				if (!sceneManager.AddSceneObject(sceneName, sceneDataPath))
				{
					F_ERROR("Failed to load scene: {}", sceneName);
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
				std::string sFilepath{ contentPath + jsonPrefab["path"].GetString() };

				if (auto pPrefab = PrefabCreator::CreatePrefab(sFilepath))
				{
					if (!assetManager.AddPrefab(sName, std::move(pPrefab)))
					{
						F_ERROR("Failed to load scene: {}", sName);
					}
				}
				else
				{
					F_ERROR("Failed to load prefab '{}' from path '{}'", sName, sFilepath);
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

	bool ProjectLoader::SaveLoadedProject(SaveProject& save)
	{
		if (!std::filesystem::exists(save.projectFilePath))
		{
			F_ERROR("Failed to save project file for '{}' at path '{}'", save.projectName, save.projectFilePath);
			return false;
		}

		std::unique_ptr<JSONSerializer> serializer{ nullptr };

		try
		{
			serializer = std::make_unique<JSONSerializer>(save.projectFilePath);
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to save tilemap '{}' - '{}'", save.projectFilePath, ex.what());
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

		serializer->StartNewObject("project_data")
			.AddKeyValuePair("project_name", save.projectName)
			.AddKeyValuePair("main_lua_script", save.mainLuaScript.substr(save.mainLuaScript.find(SCRIPTS)))
			.AddKeyValuePair("game_type", coreGlobals.GetGameTypeStr(coreGlobals.GetGameType()))
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

		std::string projectFile{ filepath + projectName + FEATHER_PRJ_FILE_EXT };
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
		auto& saveFile = MAIN_REGISTRY().GetContext<std::shared_ptr<SaveProject>>();
		saveFile->projectFilePath = projectFile;

		serializer->StartDocument();
		serializer->StartNewObject("warnings");
		serializer->AddKeyValuePair("warning", std::string{ "This file is engine generated" })
			.AddKeyValuePair("warning", std::string{ "DO NOT CHANGE unless you know what you are doing" });
		serializer->EndObject(); // Warnings

		serializer->StartNewObject("project_data")
			.AddKeyValuePair("project_name", projectName)
			.AddKeyValuePair("main_lua_file",
				saveFile->mainLuaScript.substr(saveFile->mainLuaScript.find(SCRIPTS)))
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
		std::string mainLuaFile = std::format("{}{}{}{}{}main.lua", filepath, "content", PATH_SEPARATOR, "scripts", PATH_SEPARATOR);

		auto luaSerializer = std::make_unique<LuaSerializer>(mainLuaFile);
		F_ASSERT(luaSerializer);

		// Save the main lua file path
		MAIN_REGISTRY().GetContext<std::shared_ptr<SaveProject>>()->mainLuaScript = mainLuaFile;

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

}
