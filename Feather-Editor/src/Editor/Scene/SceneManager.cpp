#include "SceneManager.h"
#include "SceneObject.h"
#include "Utils/FeatherUtilities.h"
#include "Logger/Logger.h"

#include "Editor/Tools/ToolManager.h"
#include "Editor/Tools/TileTool.h"
#include "Editor/Tools/Gizmos/Gizmo.h"
#include "Editor/Commands/CommandManager.h"

namespace Feather {

	SceneManager& SceneManager::GetInstance()
	{
		static SceneManager instance{};
		return instance;
	}

	bool SceneManager::AddScene(const std::string& sceneName)
	{
		if (m_mapScenes.contains(sceneName))
		{
			F_ERROR("Failed to add new scene object: '{}' already exists", sceneName);
			return false;
		}

		auto [itr, isSuccess] = m_mapScenes.emplace(sceneName, std::move(std::make_shared<SceneObject>(sceneName)));

		return isSuccess;
	}

	bool SceneManager::AddScene(const std::string& sceneName, const std::string& sceneData)
	{
		if (m_mapScenes.contains(sceneName))
		{
			F_ERROR("Failed to add new scene object - '{}' already exists", sceneName);
			return false;
		}

		auto [itr, success] = m_mapScenes.emplace(sceneName, std::move(std::make_shared<SceneObject>(sceneName, sceneData)));
		return success;
	}

	bool SceneManager::HasScene(const std::string& sceneName)
	{
		return m_mapScenes.contains(sceneName);
	}

	std::shared_ptr<SceneObject> SceneManager::GetScene(const std::string& sceneName)
	{
		auto sceneItr = m_mapScenes.find(sceneName);
		if (sceneItr == m_mapScenes.end())
		{
			F_ERROR("Failed to get scene object: '{}' does not exist", sceneName);
			return nullptr;
		}

		return sceneItr->second;
	}

	std::shared_ptr<SceneObject> SceneManager::GetCurrentScene()
	{
		if (m_CurrentScene.empty())
			return nullptr;

		auto sceneItr = m_mapScenes.find(m_CurrentScene);
		if (sceneItr == m_mapScenes.end())
		{
			F_ERROR("Failed to get scene object: '{}' does not exist", m_CurrentScene);
			return nullptr;
		}

		return sceneItr->second;
	}

	void SceneManager::AddLayerToCurrentScene(const std::string& layerName, bool visible)
	{
		if (auto currentScene = GetCurrentScene())
		{
			currentScene->AddLayer(layerName, visible);
			return;
		}

		F_ERROR("Failed to add layer. Current scene is nullptr");
	}

	std::vector<std::string> SceneManager::GetSceneNames() const
	{
		return GetKeys(m_mapScenes);
	}

	ToolManager& SceneManager::GetToolManager()
	{
		if (!m_ToolManager)
			m_ToolManager = std::make_unique<ToolManager>();

		F_ASSERT(m_ToolManager && "Tool manager must be valid!");
		return *m_ToolManager;
	}

	CommandManager& SceneManager::GetCommandManager()
	{
		if (!m_CommandManager)
			m_CommandManager = std::make_unique<CommandManager>();

		F_ASSERT(m_CommandManager && "Command manager must be valid!");
		return *m_CommandManager;
	}

	void SceneManager::SetTileset(const std::string& tileset)
	{
		m_CurrentTileset = tileset;
		if (!m_ToolManager)
			return;

		m_ToolManager->SetToolsCurrentTileset(tileset);
	}

	bool SceneManager::LoadCurrentScene()
	{
		if (auto currentScene = GetCurrentScene())
			return currentScene->LoadScene();

		return false;
	}

	bool SceneManager::UnloadCurrentScene()
	{
		if (auto currentScene = GetCurrentScene())
			return currentScene->UnloadScene();

		return false;
	}

	bool SceneManager::SaveAllScenes()
	{
		bool success{ true };
		for (const auto& [name, scene] : m_mapScenes)
		{
			if (!scene->SaveScene())
			{
				F_ERROR("Failed to save scene '{}'", name);
				success = false;
			}
		}

		return success;
	}

}
