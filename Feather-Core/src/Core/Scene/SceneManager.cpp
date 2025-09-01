#include "SceneManager.h"

#include "Core/Scene/Scene.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/ECS/Registry.h"
#include "Core/Loaders/TilemapLoader.h"
#include "Utils/FeatherUtilities.h"

namespace Feather {

	SceneManager::SceneManager()
		: m_mapScenes{}
		, m_CurrentScene{}
		, m_CurrentTileset{}
	{}

	bool SceneManager::AddScene(const std::string& sceneName, EMapType type)
	{
		if (m_mapScenes.contains(sceneName))
		{
			F_ERROR("Failed to add new scene object: '{}' already exists", sceneName);
			return false;
		}

		auto [itr, bSuccess] = m_mapScenes.emplace(sceneName, std::move(std::make_shared<Scene>(sceneName, type)));
		return bSuccess;
	}

	bool SceneManager::HasScene(const std::string& sceneName)
	{
		return m_mapScenes.contains(sceneName);
	}

	Scene* SceneManager::GetScene(const std::string& sceneName)
	{
		auto sceneItr = m_mapScenes.find(sceneName);
		if (sceneItr == m_mapScenes.end())
		{
			F_ERROR("Failed to get scene object: '{}' does not exist", sceneName);
			return nullptr;
		}

		return sceneItr->second.get();
	}

	Scene* SceneManager::GetCurrentScene()
	{
		if (m_CurrentScene.empty())
			return nullptr;

		auto sceneItr = m_mapScenes.find(m_CurrentScene);
		if (sceneItr == m_mapScenes.end())
		{
			F_ERROR("Failed to get scene object: '{}' does not exist", m_CurrentScene);
			return nullptr;
		}

		return sceneItr->second.get();
	}

	std::vector<std::string> SceneManager::GetSceneNames() const
	{
		return GetKeys(m_mapScenes);
	}

	bool SceneManager::LoadCurrentScene()
	{
		if (auto currentScene = GetCurrentScene())
		{
			return currentScene->LoadScene();
		}

		return false;
	}

	bool SceneManager::UnloadCurrentScene()
	{
		if (auto currentScene = GetCurrentScene())
		{
			return currentScene->UnloadScene();
		}

		return false;
	}

	bool SceneManager::CheckHasScene(const std::string& sceneName)
	{
		return m_mapScenes.contains(sceneName);
	}

	bool SceneManager::ChangeSceneName(const std::string& sOldName, const std::string& sNewName)
	{
		return KeyChange(m_mapScenes, sOldName, sNewName);
	}

	void SceneManager::CreateLuaBind(sol::state& lua, Registry& registry)
	{
		lua.new_usertype<SceneManager>(
			"SceneManager",
			sol::no_constructor,
			"setCurrentScene",
			[&registry](const std::string& sceneName)
			{
				auto* sceneManagerData = registry.TryGetContext<std::shared_ptr<SceneManagerData>>();
				if (!sceneManagerData)
				{
					F_ERROR("Scene manager data was not set correctly");
					return;
				}

				(*sceneManagerData)->sceneName = sceneName;
			},
			"changeScene",
			[&](const std::string& sceneName)
			{
				auto* sceneManagerData = registry.TryGetContext<std::shared_ptr<SceneManagerData>>();
				if (!sceneManagerData)
				{
					F_ERROR("Scene manager data was not set correctly");
					return false;
				}

				(*sceneManagerData)->sceneName = sceneName;

				sol::optional<sol::table> optSceneData = lua[sceneName + "_data"];
				if (optSceneData)
				{
					(*sceneManagerData)->defaultMusic = (*optSceneData)["default_music"].get_or(std::string{});
				}

				registry.DestroyEntities();

				TilemapLoader tl{};

				tl.LoadTilemapFromLuaTable(registry, lua[sceneName + "_tilemap"]);
				tl.LoadGameObjectsFromLuaTable(registry, lua[sceneName + "_objects"]);

				return true;
			},
			"getCanvas", // Returns the canvas of the current scene or an empty canvas object
			[&]
			{
				auto* sceneManagerData = registry.TryGetContext<std::shared_ptr<SceneManagerData>>();
				if (!sceneManagerData)
				{
					F_ERROR("Scene manager data was not set correctly");
					return Canvas{};
				}

				sol::optional<sol::table> optSceneData = lua[(*sceneManagerData)->sceneName + "_data"];
				if (optSceneData)
				{
					if (sol::optional<sol::table> optCanvas = (*optSceneData)["canvas"])
					{
						return Canvas{ .width = (*optCanvas)["width"].get_or(640),
									   .height = (*optCanvas)["height"].get_or(480),
									   .tileWidth = (*optCanvas)["tileWidth"].get_or(16),
									   .tileHeight = (*optCanvas)["tileHeight"].get_or(16) };
					}
				}

				return Canvas{};
			},
			"getDefaultMusic",
			[&]
			{
				auto* sceneManagerData = registry.TryGetContext<std::shared_ptr<SceneManagerData>>();
				if (!sceneManagerData)
				{
					F_ERROR("Scene manager data was not set correctly");
					return std::string{};
				}

				if ((*sceneManagerData)->defaultMusic.empty())
				{
					sol::optional<sol::table> optSceneData = lua[(*sceneManagerData)->sceneName + "_data"];
					if (optSceneData)
					{
						for (const auto& [key, value] : *optSceneData)
						{
							std::string sKey = key.as<std::string>();
							int x{};
						}
						(*sceneManagerData)->defaultMusic = (*optSceneData)["default_music"].get_or(std::string{});
					}
				}

				return (*sceneManagerData)->defaultMusic;
			},
			"getCurrentSceneName",
			[&]
			{
				auto* sceneManagerData = registry.TryGetContext<std::shared_ptr<SceneManagerData>>();
				if (!sceneManagerData)
				{
					F_ERROR("Scene manager data was not set correctly");
					return std::string{};
				}

				return (*sceneManagerData)->sceneName;
			}
		);
	}

}
