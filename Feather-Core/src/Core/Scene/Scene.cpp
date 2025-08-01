#include "Scene.h"

#include "Core/Loaders/TilemapLoader.h"

#include "Utils/FeatherUtilities.h"
#include "FileSystem/Serializers/JSONSerializer.h"
#include "Core/CoreUtils/ProjectInfo.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/ECS/MetaUtilities.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Character/Character.h"
#include "Core/CoreUtils/Prefab.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <format>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace Feather {

	Scene::Scene()
		: m_Registry{}
		, m_SceneName{}
		, m_TilemapPath{}
		, m_ObjectPath{}
		, m_SceneDataPath{}
		, m_DefaultMusic{}
		, m_SceneLoaded{ false }
		, m_UsePlayerStart{ false }
		, m_Canvas{}
		, m_MapType{ EMapType::Grid }
		, m_PlayerStart{ m_Registry, *this }
	{
		// NOTE: Empty scene
	}

	Scene::Scene(const std::string& sceneName, EMapType type)
		: m_Registry{}
		, m_SceneName{ sceneName }
		, m_TilemapPath{}
		, m_ObjectPath{}
		, m_SceneDataPath{}
		, m_DefaultMusic{}
		, m_SceneLoaded{ false }
		, m_UsePlayerStart{ false }
		, m_Canvas{}
		, m_MapType{ type }
		, m_PlayerStart{ m_Registry, *this }
	{
		auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
		auto optScenesPath = projectInfo->TryGetFolderPath(EProjectFolderType::Scenes);

		F_ASSERT(optScenesPath && "Scenes folder path not set correctly");

		fs::path scenePath = *optScenesPath /= m_SceneName;

		if (fs::exists(scenePath))
		{
			F_ERROR("SCENE ALREADY EXISTS!");
		}

		std::error_code ec;
		if (!fs::create_directory(scenePath, ec))
		{
			F_ERROR("Failed to create scene directory: {}", ec.message());
		}

		auto tilemapPath = scenePath / fs::path{ m_SceneName + "_tilemap.json" };
		m_TilemapPath = tilemapPath.string();
		auto objectPath = scenePath / fs::path{ m_SceneName + "_objects.json" };
		m_ObjectPath = objectPath.string();
		auto sceneDataPath = scenePath / fs::path{ m_SceneName + "_scene_data.json" };
		m_SceneDataPath = sceneDataPath.string();

		// Create the files
		std::fstream tilemap{};
		tilemap.open(m_TilemapPath, std::ios::out | std::ios::trunc);
		F_ASSERT(tilemap.is_open() && "File should have been created and opened");
		tilemap.close();

		std::fstream objectmap{};
		objectmap.open(m_ObjectPath, std::ios::out | std::ios::trunc);
		F_ASSERT(objectmap.is_open() && "File should have been created and opened");
		objectmap.close();

		std::fstream sceneData{};
		sceneData.open(m_SceneDataPath, std::ios::out | std::ios::trunc);
		F_ASSERT(sceneData.is_open() && "File should have been created and opened");
		sceneData.close();

		SaveSceneData(true);
	}

	bool Scene::LoadScene()
	{
		if (m_SceneLoaded)
		{
			F_ERROR("Scene '{}' has already been loaded", m_SceneName);
			return false;
		}

		if (!LoadSceneData())
		{
			F_ERROR("Failed to load scene data");
			return false;
		}

		// Try to load the tilemap and object maps
		auto pTilemapLoader = std::make_unique<TilemapLoader>();
		if (!pTilemapLoader->LoadTilemap(m_Registry, m_TilemapPath, true))
		{
		}

		// Load scene game objects
		if (!pTilemapLoader->LoadGameObjects(m_Registry, m_ObjectPath, true))
		{
		}

		m_SceneLoaded = true;
		F_TRACE("Loaded Scene: {}", m_SceneName);
		return true;
	}

	bool Scene::UnloadScene(bool saveScene)
	{
		if (!m_SceneLoaded)
		{
			F_ERROR("Scene '{}' has not been loaded", m_SceneName);
			return false;
		}

		if (saveScene && !SaveSceneData())
		{
			F_ERROR("Failed to unload scene data");
			return false;
		}

		// Remove all objects in registry
		m_PlayerStart.Unload();
		m_Registry.ClearRegistry();
		m_SceneLoaded = false;

		return true;
	}

	int Scene::AddLayer(const std::string& layerName, bool visible)
	{
		if (!CheckLayerName(layerName))
		{
			// F_ERROR("Failed to add layer '{}': Already exists", layerName);
			return static_cast<int>(m_LayerParams.size());
		}

		auto& spriteLayer = m_LayerParams.emplace_back(SpriteLayerParams{ .layerName = layerName, .isVisible = visible });
		spriteLayer.layer = m_LayerParams.size() - 1;

		return static_cast<int>(m_LayerParams.size());
	}

	int Scene::AddLayer(const SpriteLayerParams& layerParam)
	{
		auto layerItr = std::ranges::find_if(m_LayerParams, [&layerParam](const auto& lp) { return lp == layerParam; });

		F_ASSERT(layerItr == m_LayerParams.end() && "Layer already exists!");

		if (layerItr != m_LayerParams.end())
		{
			F_ERROR("The layer '{]' already exists", layerParam.layerName);
			return -1;
		}

		m_LayerParams.push_back(layerParam);

		std::ranges::sort(m_LayerParams, [](const auto& a, const auto& b) { return a.layer < b.layer; });

		return 1;
	}

	bool Scene::CheckLayerName(const std::string& layerName)
	{
		return CheckContainsValue(m_LayerParams, [&](SpriteLayerParams& spriteLayer)
		{
			return spriteLayer.layerName == layerName;
		});
	}

	bool Scene::LoadSceneData()
	{
		std::error_code ec;
		if (!fs::exists(m_SceneDataPath, ec))
		{
			F_ERROR("Failed to load scene data. Error: {}", ec.message());
			return false;
		}

		std::ifstream sceneDataFile;
		sceneDataFile.open(m_SceneDataPath);

		if (!sceneDataFile.is_open())
		{
			F_ERROR("Failed to open tilemap file '{}'", m_SceneDataPath);
			return false;
		}

		// The sceneData file could be empty if just created
		if (sceneDataFile.peek() == std::ifstream::traits_type::eof())
		{
			// If the sceneData is an empty file, return true. Must not have made any changes yet
			return true;
		}

		std::stringstream ss;
		ss << sceneDataFile.rdbuf();
		std::string contents = ss.str();
		rapidjson::StringStream jsonStr{ contents.c_str() };

		rapidjson::Document doc;
		doc.ParseStream(jsonStr);

		if (doc.HasParseError() || !doc.IsObject())
		{
			F_ERROR("Failed to load tilemap file '{}': is not valid JSON - {} - {}",
				m_SceneDataPath,
				rapidjson::GetParseError_En(doc.GetParseError()),
				doc.GetErrorOffset());
			return false;
		}

		F_ASSERT(doc.HasMember("scene_data") && "scene_data member is necessary");

		const rapidjson::Value& sceneData = doc["scene_data"];

		auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
		auto optScenesPath = projectInfo->TryGetFolderPath(EProjectFolderType::Scenes);
		F_ASSERT(optScenesPath && "Scenes folder path must exist");

		if (m_TilemapPath.empty())
		{
			const std::string relativeTilemap = sceneData["tilemapPath"].GetString();
			fs::path tilemapPath = *optScenesPath / relativeTilemap;
			if (!fs::exists(tilemapPath))
			{
				F_ERROR("Failed to set tilemap path: '{}' does not exist", tilemapPath.string());
				return false;
			}

			m_TilemapPath = tilemapPath.string();
		}

		if (m_ObjectPath.empty())
		{
			const std::string sRelativeObjectPath = sceneData["objectmapPath"].GetString();
			fs::path objectPath = *optScenesPath / sRelativeObjectPath;
			if (!fs::exists(objectPath))
			{
				F_ERROR("Failed to set tilemap path: '{}' does not exist", objectPath.string());
				return false;
			}
			m_ObjectPath = objectPath.string();
		}

		if (sceneData.HasMember("canvas"))
		{
			const rapidjson::Value& canvas = sceneData["canvas"];

			m_Canvas.width = canvas["width"].GetInt();
			m_Canvas.height = canvas["height"].GetInt();
			m_Canvas.tileWidth = canvas["tileWidth"].GetInt();
			m_Canvas.tileHeight = canvas["tileHeight"].GetInt();
		}

		if (sceneData.HasMember("mapType"))
		{
			std::string sMapType = sceneData["mapType"].GetString();
			if (sMapType == "grid")
			{
				m_MapType = EMapType::Grid;
			}
			else if (sMapType == "iso")
			{
				m_MapType = EMapType::IsoGrid;
				SetCanvasOffset();
			}
		}

		if (sceneData.HasMember("playerStart"))
		{
			const rapidjson::Value& playerStart = sceneData["playerStart"];
			if (playerStart.HasMember("enabled"))
			{
				m_UsePlayerStart = playerStart["enabled"].GetBool();
			}
			else
			{
				m_UsePlayerStart = false;
			}

			std::string sPlayerStartPrefab = sceneData["playerStart"]["character"].GetString();
			if (sPlayerStartPrefab != "default")
			{
				m_PlayerStart.Load(sPlayerStartPrefab);
			}

			if (m_UsePlayerStart && !m_PlayerStart.IsPlayerStartCreated())
			{
				m_PlayerStart.LoadVisualEntity();
			}

			// Do not set the position if we are not using the player start
			if (m_UsePlayerStart)
			{
				m_PlayerStart.SetPosition(glm::vec2{ sceneData["playerStart"]["position"]["x"].GetFloat(),
													 sceneData["playerStart"]["position"]["y"].GetFloat() });
			}
		}

		if (sceneData.HasMember("defaultMusic"))
		{
			m_DefaultMusic = sceneData["defaultMusic"].GetString();
		}

		F_ASSERT(sceneData.HasMember("sprite_layers") && "Sprite layers must be a part of scene data");
		const rapidjson::Value& spriteLayers = sceneData["sprite_layers"];
		for (const auto& layer : spriteLayers.GetArray())
		{
			std::string sLayerName = layer["layerName"].GetString();
			bool isVisible = layer["isVisible"].GetBool();

			AddLayer(sLayerName, isVisible);
		}

		return true;
	}

	bool Scene::SaveSceneData(bool override)
	{
		/* Scenes that have not been loaded do not need to be re-saved. They would have been
		   saved when unloading the scene previously. Only save loaded scenes */
		if (!m_SceneLoaded && !override)
		{
			return true;
		}

		// Check to see if the scene data exists
		fs::path tilemapPath{ m_SceneDataPath };
		if (!fs::exists(tilemapPath))
		{
			F_ERROR("Failed to save scene data: filepath does not exist '{}'", m_SceneDataPath);
			return false;
		}

		std::unique_ptr<JSONSerializer> pSerializer{ nullptr };

		try
		{
			pSerializer = std::make_unique<JSONSerializer>(m_SceneDataPath);
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to save scene data '{}': {}", m_SceneDataPath, ex.what());
			return false;
		}

		pSerializer->StartDocument();
		pSerializer->StartNewObject("scene_data");

		std::string sTilemapPath = m_TilemapPath.substr(m_TilemapPath.find(m_SceneName));
		std::string sObjectPath = m_ObjectPath.substr(m_ObjectPath.find(m_SceneName));

		glm::vec2 playerStartPosition = m_UsePlayerStart ? m_PlayerStart.GetPosition() : glm::vec2{ 0.0f };

		pSerializer->AddKeyValuePair("name", m_SceneName)
			.AddKeyValuePair("tilemapPath", sTilemapPath)
			.AddKeyValuePair("objectmapPath", sObjectPath)
			.AddKeyValuePair("defaultMusic", m_DefaultMusic)
			.StartNewObject("canvas")
			.AddKeyValuePair("width", m_Canvas.width)
			.AddKeyValuePair("height", m_Canvas.height)
			.AddKeyValuePair("tileWidth", m_Canvas.tileWidth)
			.AddKeyValuePair("tileHeight", m_Canvas.tileHeight)
			.EndObject() // Canvas
			.AddKeyValuePair("mapType", (m_MapType == EMapType::Grid ? std::string{ "grid" } : std::string{ "iso" }))
			.StartNewObject("playerStart")
			.AddKeyValuePair("enabled", m_UsePlayerStart)
			.AddKeyValuePair("character", m_UsePlayerStart ? m_PlayerStart.GetCharacterName() : std::string{ "default" })
			.StartNewObject("position")
			.AddKeyValuePair("x", playerStartPosition.x)
			.AddKeyValuePair("y", playerStartPosition.y)
			.EndObject() // Player start position
			.EndObject() // Player Start
			.StartNewArray("sprite_layers");

		for (const auto& layer : m_LayerParams)
		{
			pSerializer->StartNewObject()
				.AddKeyValuePair("layerName", layer.layerName)
				.AddKeyValuePair("isVisible", layer.isVisible)
				.EndObject();
		}

		pSerializer->EndArray();  // Sprite Layers
		pSerializer->EndObject(); // Scene data

		bool bSuccess{ true };
		if (!pSerializer->EndDocument())
		{
			bSuccess = false;
		}

		// Try to Save the tilemap
		auto pTilemapLoader = std::make_unique<TilemapLoader>();
		if (!pTilemapLoader->SaveTilemap(m_Registry, m_TilemapPath, true))
		{
			bSuccess = false;
		}

		// Try to Save scene game objects
		if (!pTilemapLoader->SaveGameObjects(m_Registry, m_ObjectPath, true))
		{
			bSuccess = false;
		}

		return bSuccess;
	}

	void Scene::SetCanvasOffset()
	{
		if (m_MapType == EMapType::Grid)
		{
			m_Canvas.offset = glm::vec2{ 0.0f };
			return;
		}

		// TODO: ADD checks to ensure width is 2x height for iso tiles.

		// The width of the tile must be 2x the height
		// Right now we are hard coding it everywhere;
		// however this should automatically change when in iso.
		float doubleTileWidth = m_Canvas.tileWidth * 2.0f;

		float halfTileWidth = doubleTileWidth / 2.0f;
		float halfTileHeight = m_Canvas.tileHeight / 2.0f;

		// Find the X Offset
		// Get the Hypotenuse of one tile
		float c1 = sqrt((halfTileWidth * halfTileWidth) + (halfTileHeight * halfTileHeight));

		// Get the length of the entire Y - Side hypotenuse
		float c2 = m_Canvas.width / doubleTileWidth * c1;

		// Find theta
		float theta1 = atan2(halfTileWidth, halfTileHeight);

		// Get the length of offset x
		float offsetX = sin(theta1) * c2;
		m_Canvas.offset.x = floor(offsetX);
		// We are assuming that there is no offset in Y currently
		m_Canvas.offset.y = 0.0f;
	}

	void Scene::CreateLuaBind(sol::state& lua)
	{
		lua.new_usertype<Canvas>("Canvas",
			sol::call_constructor,
			sol::factories([] { return Canvas{}; },
				[](int width, int height, int tileWidth, int tileHeight)
				{
					return Canvas{ .width = width,
								   .height = height,
								   .tileWidth = tileWidth,
								   .tileHeight = tileHeight
					};
				}),
			"width",
			&Canvas::width,
			"height",
			&Canvas::height,
			"tileWidth",
			&Canvas::tileWidth,
			"tileHeight",
			&Canvas::tileHeight);
	}

}
