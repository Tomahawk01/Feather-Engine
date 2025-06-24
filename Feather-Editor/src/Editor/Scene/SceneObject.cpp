#include "SceneObject.h"

#include "Utils/FeatherUtilities.h"
#include "Core/ECS/MetaUtilities.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Loaders/TilemapLoader.h"
#include "Core/Events/EventDispatcher.h"
#include "Core/CoreUtils/SaveProject.h"
#include "Filesystem/Serializers/JSONSerializer.h"
#include "Utils/FeatherUtilities.h"

#include "Editor/Events/EditorEventTypes.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <filesystem>
#include <fstream>
#include <format>

using namespace entt::literals;

namespace Feather {

	SceneObject::SceneObject(const std::string& sceneName, EMapType type)
		: Scene(sceneName, type)
		, m_RuntimeRegistry{}
		, m_RuntimeSceneName{ "" }
		, m_CurrentLayer{ 0 }
	{
		ADD_EVENT_HANDLER(NameChangeEvent, &SceneObject::OnEntityNameChanges, *this);
	}

	SceneObject::SceneObject(const std::string& sceneName, const std::string& sceneData)
		: m_RuntimeRegistry{}
		, m_CurrentLayer{ 0 }
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
		m_RuntimeSceneName = m_SceneName;

		auto& registryToCopy = m_Registry.GetRegistry();

		for (auto entityToCopy : registryToCopy.view<entt::entity>(entt::exclude<ScriptComponent, UneditableComponent>))
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

		if (m_UsePlayerStart)
			m_PlayerStart.CreatePlayer(m_RuntimeRegistry);
	}

	void SceneObject::CopySceneToRuntime(SceneObject& sceneToCopy)
	{
		m_RuntimeSceneName = sceneToCopy.GetSceneName();

		auto& registry = sceneToCopy.GetRegistry();
		auto& registryToCopy = registry.GetRegistry();

		for (auto entityToCopy : registryToCopy.view<entt::entity>(entt::exclude<ScriptComponent, UneditableComponent>))
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
		m_RuntimeSceneName.clear();
	}

	void SceneObject::AddNewLayer()
	{
		m_LayerParams.emplace_back(SpriteLayerParams{ .layerName = std::format("NewLayer_{}", m_CurrentLayer++) });
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
		bool sceneLoaded = Scene::LoadScene();

		// Map the entities
		auto view = m_Registry.GetRegistry().view<entt::entity>(entt::exclude<TileComponent>);
		for (auto entity : view)
		{
			Entity ent{ m_Registry, entity };
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
