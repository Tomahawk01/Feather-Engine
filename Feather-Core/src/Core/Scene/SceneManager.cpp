#include "SceneManager.h"

#include "Core/Scene/Scene.h"
#include "Utils/FeatherUtilities.h"

namespace Feather {

	SceneManager::SceneManager()
		: m_mapScenes{}
		, m_CurrentScene{ "" }
		, m_CurrentTileset{ "" }
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
		if (auto pCurrentScene = GetCurrentScene())
		{
			return pCurrentScene->LoadScene();
		}

		return false;
	}

	bool SceneManager::UnloadCurrentScene()
	{
		if (auto pCurrentScene = GetCurrentScene())
		{
			return pCurrentScene->UnloadScene();
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

}
