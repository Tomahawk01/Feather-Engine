#include "SceneObject.h"

#include "Utils/FeatherUtilities.h"
#include "Core/ECS/MetaUtilities.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Loaders/TilemapLoader.h"
#include "Core/Events/EventDispatcher.h"
#include "Filesystem/Serializers/JSONSerializer.h"
#include "Utils/FeatherUtilities.h"

#include "Editor/Utilities/SaveProject.h"
#include "Editor/Events/EditorEventTypes.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <filesystem>
#include <fstream>
#include <format>

using namespace entt::literals;

namespace Feather {

	SceneObject::SceneObject(const std::string& sceneName)
		: m_Registry{}
		, m_RuntimeRegistry{}
		, m_SceneName{ sceneName }
		, m_RuntimeSceneName{ "" }
		, m_TilemapPath{ "" }
		, m_ObjectPath{ "" }
		, m_SceneDataPath{ "" }
		, m_Canvas{}
		, m_CurrentLayer{ 0 }
		, m_SceneLoaded{ false }
	{
		auto& pSaveProject = MAIN_REGISTRY().GetContext<std::shared_ptr<SaveProject>>();
		F_ASSERT(pSaveProject && "SaveProject must exists here!");

		std::string scenePath = std::format("{}content{}assets{}scenes{}{}", pSaveProject->projectPath, PATH_SEPARATOR, PATH_SEPARATOR, PATH_SEPARATOR, m_SceneName);

		if (std::filesystem::exists(scenePath))
			F_ERROR("Scene already exists!");

		std::error_code ec;
		if (!std::filesystem::create_directory(std::filesystem::path{ scenePath }, ec))
			F_ERROR("Failed to create scene directory '{}': ", ec.message());

		m_TilemapPath = scenePath + PATH_SEPARATOR + m_SceneName + "_tilemap.json";
		m_ObjectPath = scenePath + PATH_SEPARATOR + m_SceneName + "_objects.json";
		m_SceneDataPath = scenePath + PATH_SEPARATOR + m_SceneName + "_scene_data.json";

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

		ADD_EVENT_HANDLER(NameChangeEvent, &SceneObject::OnEntityNameChanges, *this);
	}

	SceneObject::SceneObject(const std::string& sceneName, const std::string& sceneData)
		: m_Registry{}
		, m_RuntimeRegistry{}
		, m_SceneName{ sceneName }
		, m_TilemapPath{}
		, m_ObjectPath{}
		, m_SceneDataPath{ sceneData }
		, m_Canvas{}
		, m_CurrentLayer{ 0 }
		, m_SceneLoaded{ false }
	{
		// We need to load the scene data from the json file!
		if (!LoadSceneData())
		{
			F_ERROR("Failed to load scene data");
			return;
		}

		// Verify that the tilemap and objectmap files exist
		if (!std::filesystem::exists(std::filesystem::path{ m_TilemapPath }))
		{
			// Log some error
		}

		if (!std::filesystem::exists(std::filesystem::path{ m_ObjectPath }))
		{
			// Log some error
		}

		ADD_EVENT_HANDLER(NameChangeEvent, &SceneObject::OnEntityNameChanges, *this);
	}

	void SceneObject::CopySceneToRuntime()
	{
		m_RuntimeSceneName = m_SceneName;

		auto& registryToCopy = m_Registry.GetRegistry();

		for (auto entityToCopy : registryToCopy.view<entt::entity>(entt::exclude<ScriptComponent>))
		{
			entt::entity newEntity = m_RuntimeRegistry.CreateEntity();

			// Copy components of the entity to the new entity
			for (auto&& [id, storage] : registryToCopy.storage())
			{
				if (!storage.contains(entityToCopy))
					continue;

				InvokeMetaFunction(id, "copy_component"_hs, Entity{ m_Registry, entityToCopy }, Entity{ m_RuntimeRegistry, newEntity });
			}
		}
	}

	void SceneObject::CopySceneToRuntime(SceneObject& sceneToCopy)
	{
		m_RuntimeSceneName = sceneToCopy.GetSceneName();

		auto& registry = sceneToCopy.GetRegistry();
		auto& registryToCopy = registry.GetRegistry();

		for (auto entityToCopy : registryToCopy.view<entt::entity>(entt::exclude<ScriptComponent>))
		{
			entt::entity newEntity = m_RuntimeRegistry.CreateEntity();

			// Copy the components of the entity to the new entity
			for (auto&& [id, storage] : registryToCopy.storage())
			{
				if (!storage.contains(entityToCopy))
					continue;

				InvokeMetaFunction(id, "copy_component"_hs, Entity{ registry, entityToCopy }, Entity{ m_RuntimeRegistry, newEntity });
			}
		}
	}

	void SceneObject::ClearRuntimeScene()
	{
		m_RuntimeRegistry.ClearRegistry();
		m_RuntimeSceneName.clear();
	}

	void SceneObject::AddNewLayer()
	{
		m_LayerParams.emplace_back(SpriteLayerParams{ .layerName = std::format("NewLayer_{}", m_CurrentLayer++) });
	}

	void SceneObject::AddLayer(const std::string& layerName, bool visible)
	{
		if (!CheckLayerName(layerName))
		{
			// F_ERROR( "Failed to add layer '{}': Already exists", layerName );
			return;
		}

		m_LayerParams.emplace_back(SpriteLayerParams{ .layerName = layerName, .isVisible = visible });
	}

	bool SceneObject::CheckLayerName(const std::string& layerName)
	{
		return CheckContainsValue(m_LayerParams,
			[&](SpriteLayerParams& spriteLayer) { return spriteLayer.layerName == layerName; }
		);
	}

	bool SceneObject::AddGameObject()
	{
		Entity newObject{ m_Registry, "", "" };
		newObject.AddComponent<TransformComponent>();
		std::string tag{ "GameObject" };

		auto objItr = m_mapTagToEntity.find(tag);
		if (objItr != m_mapTagToEntity.end())
		{
			size_t objIndex{ 1 };
			tag = "GameObject1";
			objItr = m_mapTagToEntity.find(tag);
			while (objItr != m_mapTagToEntity.end())
			{
				++objIndex;
				tag = std::format("GameObject{}", objIndex);
				objItr = m_mapTagToEntity.find(tag);
			}
		}

		newObject.ChangeName(tag);
		m_mapTagToEntity.emplace(tag, newObject.GetEntity());

		return true;
	}

	bool SceneObject::AddGameObjectByTag(const std::string& tag, entt::entity entity)
	{
		F_ASSERT(entity != entt::null && "The entity passed in must be valid");

		if (m_mapTagToEntity.contains(tag))
		{
			F_ERROR("Failed to add entity with tag '{}': Already exists!", tag);
			return false;
		}

		m_mapTagToEntity.emplace(tag, entity);

		return true;
	}

	bool SceneObject::DuplicateGameObject(entt::entity entity)
	{
		auto objItr = m_mapTagToEntity.begin();
		for (; objItr != m_mapTagToEntity.end(); ++objItr)
		{
			if (objItr->second == entity)
				break;
		}

		if (objItr == m_mapTagToEntity.end())
		{
			F_ERROR("Failed to duplicate game object with id '{}'. Does not exist or was not mapped correctly", static_cast<uint32_t>(entity));
			return false;
		}

		// Create the new entity in the registry
		auto& registry = m_Registry.GetRegistry();
		auto newEntity = registry.create();

		// Copy the components of the entity to the new entity
		for (auto&& [id, storage] : registry.storage())
		{
			if (!storage.contains(entity))
				continue;

			InvokeMetaFunction(id, "copy_component"_hs, Entity{ m_Registry, entity }, Entity{ m_Registry, newEntity });
		}

		// Now we need to set the tag for the entity
		size_t tagNum{ 1 };

		while (CheckTagName(std::format("{}_{}", objItr->first, tagNum)))
		{
			++tagNum;
		}

		Entity newEnt{ m_Registry, newEntity };
		newEnt.ChangeName(std::format("{}_{}", objItr->first, tagNum));

		m_mapTagToEntity.emplace(newEnt.GetName(), newEntity);

		return true;
	}

	bool SceneObject::DeleteGameObjectByTag(const std::string& tag)
	{
		auto objItr = m_mapTagToEntity.find(tag);
		if (objItr == m_mapTagToEntity.end())
		{
			F_ERROR("Failed to delete game object with tag '{}'. Does not exist or was not mapped correctly", tag);
			return false;
		}

		std::vector<std::string> removedEntities;
		Entity ent{ m_Registry, objItr->second };

		RelationshipUtils::RemoveAndDelete(ent, removedEntities);

		for (const auto& sTag : removedEntities)
			m_mapTagToEntity.erase(sTag);

		return true;
	}

	bool SceneObject::DeleteGameObjectById(entt::entity entity)
	{
		auto objItr = m_mapTagToEntity.begin();
		for (; objItr != m_mapTagToEntity.end(); ++objItr)
		{
			if (objItr->second == entity)
				break;
		}

		if (objItr == m_mapTagToEntity.end())
		{
			F_ERROR("Failed to delete game object with id '{}'. Does not exist or was not mapped correctly", static_cast<uint32_t>(entity));
			return false;
		}

		std::vector<std::string> removedEntities;
		Entity ent{ m_Registry, objItr->second };

		RelationshipUtils::RemoveAndDelete(ent, removedEntities);

		for (const auto& sTag : removedEntities)
			m_mapTagToEntity.erase(sTag);

		return true;
	}

	bool SceneObject::LoadScene()
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
		auto tilemapLoader = std::make_unique<TilemapLoader>();
		if (!tilemapLoader->LoadTilemap(m_Registry, m_TilemapPath, true))
		{
		}

		// Load scene game objects
		if (!tilemapLoader->LoadGameObjects(m_Registry, m_ObjectPath, true))
		{
		}

		// Map the entities
		auto view = m_Registry.GetRegistry().view<entt::entity>(entt::exclude<TileComponent>);
		for (auto entity : view)
		{
			Entity ent{ m_Registry, entity };
			AddGameObjectByTag(ent.GetName(), entity);
		}

		m_SceneLoaded = true;

		return true;
	}

	bool SceneObject::UnloadScene(bool saveScene)
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
		m_Registry.ClearRegistry();
		m_mapTagToEntity.clear();
		m_SceneLoaded = false;

		return false;
	}

	bool SceneObject::SaveScene()
	{
		return SaveSceneData();
	}

	bool SceneObject::CheckTagName(const std::string& tagName)
	{
		return m_mapTagToEntity.contains(tagName);
	}

	bool SceneObject::LoadSceneData()
	{
		std::error_code ec;
		if (!std::filesystem::exists(m_SceneDataPath, ec))
		{
			F_ERROR("Failed to load scene data: {}", ec.message());
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
			// If the sceneData is an empty file, return true. must not have made any changes yet
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
			F_ERROR("Failed to load tilemap: File '{}' is not valid JSON - {} - {}", m_SceneDataPath, rapidjson::GetParseError_En(doc.GetParseError()), doc.GetErrorOffset());
			return false;
		}

		F_ASSERT(doc.HasMember("scene_data") && "scene_data member is necessary");

		const rapidjson::Value& sceneData = doc["scene_data"];

		auto& saveProject = MAIN_REGISTRY().GetContext<std::shared_ptr<SaveProject>>();
		F_ASSERT(saveProject && "SaveProject must exists here!");

		std::string scenePath = std::format("{}content{}", saveProject->projectPath, PATH_SEPARATOR);

		if (m_TilemapPath.empty())
			m_TilemapPath = scenePath + sceneData["tilemapPath"].GetString();

		if (m_ObjectPath.empty())
			m_ObjectPath = scenePath + sceneData["objectmapPath"].GetString();

		if (sceneData.HasMember("canvas"))
		{
			const rapidjson::Value& canvas = sceneData["canvas"];

			m_Canvas.width = canvas["width"].GetInt();
			m_Canvas.height = canvas["height"].GetInt();
			m_Canvas.tileWidth = canvas["tileWidth"].GetInt();
			m_Canvas.tileHeight = canvas["tileHeight"].GetInt();
		}

		F_ASSERT(sceneData.HasMember("sprite_layers") && "Sprite layers must be a part of scene data");
		const rapidjson::Value& spriteLayers = sceneData["sprite_layers"];
		for (const auto& layer : spriteLayers.GetArray())
		{
			std::string layerName = layer["layerName"].GetString();
			bool visible = layer["isVisible"].GetBool();
			AddLayer(layerName, visible);
		}

		return true;
	}

	bool SceneObject::SaveSceneData()
	{
		// Scenes that have not been loaded do not need to be re-saved.
		// They would have been saved when unloading the scene previously.
		// Only save loaded scenes
		if (!m_SceneLoaded)
			return true;

		// Check to see if the scene data exists
		std::filesystem::path tilemapPath{ m_SceneDataPath };
		if (!std::filesystem::exists(tilemapPath))
		{
			F_ERROR("Failed to save scene data: Filepath does not exist '{}'", m_SceneDataPath);
			return false;
		}

		std::unique_ptr<JSONSerializer> serializer{ nullptr };

		try
		{
			serializer = std::make_unique<JSONSerializer>(m_SceneDataPath);
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to save scene data '{}': '{}'", m_SceneDataPath, ex.what());
			return false;
		}

		serializer->StartDocument();
		serializer->StartNewObject("scene_data");

		std::string sTilemapPath = m_TilemapPath.substr(m_TilemapPath.find(ASSETS));
		std::string objectPath = m_ObjectPath.substr(m_ObjectPath.find(ASSETS));

		serializer->AddKeyValuePair("name", m_SceneName)
			.AddKeyValuePair("tilemapPath", sTilemapPath)
			.AddKeyValuePair("objectmapPath", objectPath)
			.StartNewObject("canvas")
			.AddKeyValuePair("width", m_Canvas.width)
			.AddKeyValuePair("height", m_Canvas.height)
			.AddKeyValuePair("tileWidth", m_Canvas.tileWidth)
			.AddKeyValuePair("tileHeight", m_Canvas.tileHeight)
			.EndObject() // Canvas
			.StartNewArray("sprite_layers");

		for (const auto& layer : m_LayerParams)
		{
			serializer->StartNewObject()
				.AddKeyValuePair("layerName", layer.layerName)
				.AddKeyValuePair("isVisible", layer.isVisible)
				.EndObject();
		}

		serializer->EndArray();  // Sprite Layers
		serializer->EndObject(); // Scene data

		bool success{ true };
		if (!serializer->EndDocument())
			success = false;

		// Try to Save the tilemap
		auto tilemapLoader = std::make_unique<TilemapLoader>();
		if (!tilemapLoader->SaveTilemap(m_Registry, m_TilemapPath, true))
			success = false;

		// Try to Save scene game objects
		if (!tilemapLoader->SaveGameObjects(m_Registry, m_ObjectPath, true))
			success = false;

		return success;
	}

	void SceneObject::OnEntityNameChanges(NameChangeEvent& nameChange)
	{
		if (nameChange.newName.empty() || nameChange.oldName.empty() || !nameChange.entity)
			return;

		auto objItr = m_mapTagToEntity.find(nameChange.oldName);
		if (objItr == m_mapTagToEntity.end())
			return;

		// If the map already contains that name, don't change the name
		if (m_mapTagToEntity.contains(nameChange.newName))
		{
			nameChange.entity->ChangeName(nameChange.oldName);
			F_ERROR("Failed to change entity name. '{}' already exists", nameChange.newName);
			return;
		}

		if (nameChange.entity->GetEntity() != objItr->second)
			return;

		if (!KeyChange(m_mapTagToEntity, nameChange.oldName, nameChange.newName))
		{
			F_ERROR("Failed to change entity name");
			return;
		}
	}

}