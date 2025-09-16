#include "SceneObject.h"

#include "Utils/FeatherUtilities.h"
#include "Core/ECS/MetaUtilities.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Loaders/TilemapLoader.h"
#include "Core/Events/EventDispatcher.h"
#include "Core/CoreUtils/ProjectInfo.h"
#include "FileSystem/Serializers/JSONSerializer.h"
#include "FileSystem/Serializers/LuaSerializer.h"

#include "Editor/Events/EditorEventTypes.h"
#include "Editor/Commands/CommandManager.h"
#include "Editor/Scene/SceneManager.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

using namespace entt::literals;

namespace Feather {

	SceneObject::SceneObject(const std::string& sceneName, EMapType type)
		: Scene(sceneName, type)
		, m_RuntimeRegistry{}
		, m_RuntimeData{ nullptr }
	{
		ADD_EVENT_HANDLER(NameChangeEvent, &SceneObject::OnEntityNameChanges, *this);
	}

	SceneObject::SceneObject(const std::string& sceneName, const std::string& sceneData)
		: m_RuntimeRegistry{}
	{
		m_SceneName = sceneName;
		m_SceneDataPath = sceneData;

		// We need to load the scene data from the json file!
		if (!LoadSceneData())
		{
			F_ERROR("Failed to load scene data");
			return;
		}

		// Verify that the tilemap and objectmap files exist
		if (!std::filesystem::exists(std::filesystem::path{ m_TilemapPath }))
		{
			F_WARN("Tilemap file '{}' does not exist", m_TilemapPath);
		}

		if (!std::filesystem::exists(std::filesystem::path{ m_ObjectPath }))
		{
			F_WARN("Object file '{}' does not exist", m_ObjectPath);
		}

		ADD_EVENT_HANDLER(NameChangeEvent, &SceneObject::OnEntityNameChanges, *this);
	}

	void SceneObject::CopySceneToRuntime()
	{
		if (!m_RuntimeData)
		{
			m_RuntimeData = std::make_unique<SceneRuntimeData>();
		}

		// Setup runtime data
		m_RuntimeData->canvas = m_Canvas;
		m_RuntimeData->defaultMusic = m_DefaultMusic;
		m_RuntimeData->sceneName = m_SceneName;

		auto& registryToCopy = m_Registry.GetRegistry();

		for (auto entityToCopy : registryToCopy.view<entt::entity>(entt::exclude<ScriptComponent, UneditableComponent>))
		{
			entt::entity newEntity = m_RuntimeRegistry.CreateEntity();

			// Copy components of the entity to the new entity
			for (auto&& [id, storage] : registryToCopy.storage())
			{
				if (!storage.contains(entityToCopy))
					continue;

				InvokeMetaFunction(id, "copy_component"_hs, Entity{ &m_Registry, entityToCopy }, Entity{ &m_RuntimeRegistry, newEntity });
			}
		}

		if (m_UsePlayerStart)
			m_PlayerStart.CreatePlayer(m_RuntimeRegistry);
	}

	void SceneObject::CopySceneToRuntime(SceneObject& sceneToCopy)
	{
		if (!m_RuntimeData)
		{
			m_RuntimeData = std::make_unique<SceneRuntimeData>();
		}

		// Setup runtime data
		m_RuntimeData->canvas = sceneToCopy.GetCanvas();
		m_RuntimeData->defaultMusic = sceneToCopy.GetDefaultMusicName();
		m_RuntimeData->sceneName = sceneToCopy.GetSceneName();

		auto& registry = sceneToCopy.GetRegistry();
		auto& registryToCopy = registry.GetRegistry();

		m_RuntimeRegistry.DestroyEntities();

		for (auto entityToCopy : registryToCopy.view<entt::entity>(entt::exclude<UneditableComponent>))
		{
			entt::entity newEntity = m_RuntimeRegistry.CreateEntity();

			// Copy the components of the entity to the new entity
			for (auto&& [id, storage] : registryToCopy.storage())
			{
				if (!storage.contains(entityToCopy))
					continue;

				InvokeMetaFunction(id, "copy_component"_hs, Entity{ &registry, entityToCopy }, Entity{ &m_RuntimeRegistry, newEntity });
			}
		}

		// Copy the player start from the new scene
		if (sceneToCopy.IsPlayerStartEnabled())
			sceneToCopy.CopyPlayerStartToRuntimeRegistry(m_RuntimeRegistry);
	}

	void SceneObject::CopyPlayerStartToRuntimeRegistry(Registry& runtimeRegistry)
	{
		if (!m_UsePlayerStart)
		{
			F_ERROR("Failed to copy player to runtime. Not enabled");
			return;
		}

		m_PlayerStart.CreatePlayer(runtimeRegistry);
	}

	void SceneObject::ClearRuntimeScene()
	{
		m_RuntimeRegistry.ClearRegistry();
		m_RuntimeData.reset();
	}

	void SceneObject::AddNewLayer()
	{
		bool checkLayer{ true };
		size_t currentLayer{ 0 };
		while (checkLayer)
		{
			auto hasLayerItr = std::ranges::find_if(m_LayerParams,
			[&](const auto& layerParam)
			{
				return layerParam.layerName == std::format("NewLayer_{}", currentLayer);
			});

			if (hasLayerItr != m_LayerParams.end())
			{
				++currentLayer;
			}
			else
			{
				checkLayer = false;
			}
		}

		const SpriteLayerParams spriteLayerParams{ .layerName = std::format("NewLayer_{}", currentLayer), .layer = static_cast<int>(currentLayer) };

		m_LayerParams.emplace_back(spriteLayerParams);

		auto addTileLayerCmd = UndoableCommands{ AddTileLayerCmd{.sceneObject = this, .spriteLayerParams = spriteLayerParams} };

		COMMAND_MANAGER().Execute(addTileLayerCmd);
	}

	bool SceneObject::DeleteLayer(int layer)
	{
		F_ASSERT(!m_LayerParams.empty() && "Layer params should not be empty");
		F_ASSERT(m_LayerParams.size() > layer && "Layer is outside of the range");

		auto copySpriteLayer = m_LayerParams[layer];

		m_LayerParams.erase(m_LayerParams.begin() + layer);

		// Drop all layers above removed layer down by 1
		for (auto& spriteLayer : m_LayerParams)
		{
			if (spriteLayer.layer > layer)
			{
				spriteLayer.layer -= 1;
			}
		}

		std::vector<Tile> removedTiles{};
		auto view = m_Registry.GetRegistry().view<TileComponent, SpriteComponent>();
		for (auto entity : view)
		{
			Entity ent{ &m_Registry, entity };
			auto& sprite = ent.GetComponent<SpriteComponent>();
			if (sprite.layer == layer)
			{
				Tile removedTile{};
				removedTile.transform = ent.GetComponent<TransformComponent>();
				removedTile.sprite = ent.GetComponent<SpriteComponent>();

				if (auto* boxCollider = ent.TryGetComponent<BoxColliderComponent>())
				{
					removedTile.isCollider = true;
					removedTile.boxCollider = *boxCollider;
				}

				if (auto* circleCollider = ent.TryGetComponent<CircleColliderComponent>())
				{
					removedTile.isCircle = true;
					removedTile.circleCollider = *circleCollider;
				}

				if (auto* animation = ent.TryGetComponent<AnimationComponent>())
				{
					removedTile.hasAnimation = true;
					removedTile.animation = *animation;
				}

				if (auto* physics = ent.TryGetComponent<PhysicsComponent>())
				{
					removedTile.hasPhysics = true;
					removedTile.physics = *physics;
				}

				ent.Destroy();
				removedTiles.push_back(removedTile);
			}
			else if (sprite.layer > layer) // Drop the layer down if greater
			{
				sprite.layer -= 1;
			}
		}

		auto removeTileLayerCmd = UndoableCommands{ RemoveTileLayerCmd{.sceneObject = this, .tilesRemoved = removedTiles, .spriteLayerParams = copySpriteLayer} };

		COMMAND_MANAGER().Execute(removeTileLayerCmd);

		return true;
	}

	bool SceneObject::AddGameObject()
	{
		Entity newObject{ &m_Registry, "", "" };
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

			InvokeMetaFunction(id, "copy_component"_hs, Entity{ &m_Registry, entity }, Entity{ &m_Registry, newEntity });
		}

		// Now we need to set the tag for the entity
		size_t tagNum{ 1 };

		while (CheckTagName(std::format("{}_{}", objItr->first, tagNum)))
		{
			++tagNum;
		}

		Entity newEnt{ &m_Registry, newEntity };
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
		Entity ent{ &m_Registry, objItr->second };

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
		Entity ent{ &m_Registry, objItr->second };

		RelationshipUtils::RemoveAndDelete(ent, removedEntities);

		for (const auto& sTag : removedEntities)
			m_mapTagToEntity.erase(sTag);

		return true;
	}

	bool SceneObject::LoadScene()
	{
		bool sceneLoaded = Scene::LoadScene();

		// Map the entities
		auto view = m_Registry.GetRegistry().view<entt::entity>(entt::exclude<TileComponent>);
		for (auto entity : view)
		{
			Entity ent{ &m_Registry, entity };
			AddGameObjectByTag(ent.GetName(), entity);
		}

		return sceneLoaded;
	}

	bool SceneObject::UnloadScene(bool saveScene)
	{
		bool success = Scene::UnloadScene(saveScene);
		m_mapTagToEntity.clear();

		return success;
	}

	SceneExportFiles SceneObject::ExportSceneToLua(const std::string& sceneName, const std::string& exportPath, Registry& registry)
	{
		if (!fs::exists(m_SceneDataPath))
		{
			F_ERROR("Failed to export scene. Scene data does not exist");
			return {};
		}

		std::ifstream sceneDataFile;
		sceneDataFile.open(m_SceneDataPath);

		if (!sceneDataFile.is_open())
		{
			F_ERROR("Failed to open scene data file '{}'", m_SceneDataPath);
			return {};
		}

		if (sceneDataFile.peek() == std::ifstream::traits_type::eof())
		{
			return {};
		}

		std::stringstream ss;
		ss << sceneDataFile.rdbuf();
		std::string contents = ss.str();

		rapidjson::StringStream jsonStr{ contents.c_str() };

		rapidjson::Document doc;
		doc.ParseStream(jsonStr);

		if (doc.HasParseError() || !doc.IsObject())
		{
			F_ERROR("Failed to export scene data. File '{}' is not a valid json. Error: {}, Offset: {}", m_SceneDataPath, rapidjson::GetParseError_En(doc.GetParseError()), doc.GetErrorOffset());
			return {};
		}

		F_ASSERT(doc.HasMember("scene_data") && "scene_data member is necessary");

		const rapidjson::Value& sceneData = doc["scene_data"];

		auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
		auto optScenesPath = projectInfo->TryGetFolderPath(EProjectFolderType::Scenes);

		F_ASSERT(optScenesPath && "Scene folder must exist");

		if (m_TilemapPath.empty())
		{
			m_TilemapPath = fs::path{ *optScenesPath / sceneData["tilemapPath"].GetString() }.string();
		}

		if (m_ObjectPath.empty())
		{
			m_ObjectPath = fs::path{ *optScenesPath / sceneData["objectmapPath"].GetString() }.string();
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
			std::string mapType = sceneData["mapType"].GetString();
			if (mapType == "grid")
			{
				m_MapType = EMapType::Grid;
			}
			else if (mapType == "iso")
			{
				m_MapType = EMapType::IsoGrid;
			}
		}

		auto tilemapLoader = std::make_unique<TilemapLoader>();
		if (!tilemapLoader->LoadTilemap(registry, m_TilemapPath, true))
		{
			F_ERROR("Failed to load tilemap '{}'", m_TilemapPath);
			return {};
		}

		if (!tilemapLoader->LoadGameObjects(registry, m_ObjectPath, true))
		{
			F_ERROR("Failed to load game object map '{}'", m_ObjectPath);
			return {};
		}

		if (IsPlayerStartEnabled())
		{
			CopyPlayerStartToRuntimeRegistry(registry);
		}

		fs::path exportPath_{ exportPath };
		if (!fs::exists(exportPath_))
		{
			F_ERROR("Faield to export scene '{}' to lua. Export path '{}' does not exist", sceneName, exportPath);
			return {};
		}

		fs::path tilemapLua{ exportPath_ };
		tilemapLua /= sceneName + "_tilemap.lua";

		if (!tilemapLoader->SaveTilemap(registry, tilemapLua.string(), false))
		{
			F_ERROR("Failed to export scene '{}' to lua", sceneName);
			return {};
		}

		fs::path objectLua{ exportPath_ };
		objectLua /= sceneName + "_objects.lua";

		if (!tilemapLoader->SaveGameObjects(registry, objectLua.string(), false))
		{
			F_ERROR("Failed to export scene '{}' to lua", sceneName);
			return {};
		}

		// Export Scene Data
		std::unique_ptr<LuaSerializer> serializer{ nullptr };

		fs::path sceneDataPath{ exportPath };
		sceneDataPath /= sceneName + "_data.lua";

		try
		{
			serializer = std::make_unique<LuaSerializer>(sceneDataPath.string());
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to save tilemap '{}': {}", sceneDataPath.string(), ex.what());
			return {};
		}

		if (m_DefaultMusic.empty() && sceneData.HasMember("defaultMusic"))
		{
			m_DefaultMusic = sceneData["defaultMusic"].GetString();
		}

		serializer->StartNewTable(sceneName + "_data")
			.AddKeyValuePair("default_music", m_DefaultMusic, true, false, false, true)
			.AddKeyValuePair("scene_name", m_SceneName, true, false, false, true)
			.StartNewTable("canvas")
			.AddKeyValuePair("width", m_Canvas.width)
			.AddKeyValuePair("height", m_Canvas.height)
			.AddKeyValuePair("tileWidth", m_Canvas.tileWidth)
			.AddKeyValuePair("tileHeight", m_Canvas.tileHeight, true, true)
			.EndTable()	 // canvas
			.EndTable(); // _data
		serializer->FinishStream();

		return { tilemapLua.string(), objectLua.string(), sceneDataPath.string() };
	}

	bool SceneObject::CheckTagName(const std::string& tagName)
	{
		return m_mapTagToEntity.contains(tagName);
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
