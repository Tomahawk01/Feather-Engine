#include "SceneManager.h"

#include "Core/Scene/Scene.h"
#include "Core/ECS/Components/AllComponents.h"
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

	void SceneManager::CreateLuaBind(sol::state& lua, SceneManager& sceneManager)
	{
		lua.new_usertype<SceneManager>(
			"SceneManager",
			sol::no_constructor,
			"changeScene",
			// TODO: This will still need testing once the runtime has been created
			[&](const std::string& sceneName)
			{
				auto currentScene = sceneManager.GetCurrentScene();
				if (!currentScene)
				{
					F_ERROR("Failed to change to scene '{}': Current scene is invalid", sceneName);
					return false;
				}

				if (currentScene->GetSceneName() == sceneName)
				{
					F_ERROR("Failed to load scene '{}': Scene has already been loaded", sceneName);
					return false;
				}

				auto pScene = sceneManager.GetScene(sceneName);
				if (!pScene)
				{
					F_ERROR("Failed to change to scene '{}': Scene is invalid", sceneName);
					return false;
				}

				currentScene->GetRegistry().DestroyEntities<ScriptComponent>();
				currentScene->UnloadScene();

				if (!pScene->IsLoaded())
				{
					pScene->LoadScene();
				}

				sceneManager.SetCurrentScene(sceneName);

				return true;
			},
			"getCanvas", // Returns the canvas of the current scene or an empty canvas object
			[&]
			{
				if (auto currentScene = sceneManager.GetCurrentScene())
					return currentScene->GetCanvas();

				return Canvas{};
			},
			"getDefaultMusic",
			[&]
			{
				if (auto currentScene = sceneManager.GetCurrentScene())
					return currentScene->GetDefaultMusicName();

				return std::string{};
			},
			"getCurrentSceneName", [&] { return sceneManager.GetCurrentSceneName(); }
		);
	}

}
