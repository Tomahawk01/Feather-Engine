#include "SceneManager.h"

#include "SceneObject.h"

#include "Logger/Logger.h"
#include "Core/Events/EventDispatcher.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/CoreUtils/SaveProject.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Utils/FeatherUtilities.h"

#include "Editor/Tools/ToolManager.h"
#include "Editor/Tools/TileTool.h"
#include "Editor/Tools/Gizmos/Gizmo.h"
#include "Editor/Commands/CommandManager.h"
#include "Editor/Loaders/ProjectLoader.h"

#include <filesystem>

namespace Feather {

	EditorSceneManager& EditorSceneManager::GetInstance()
	{
		static EditorSceneManager instance{};
		return instance;
	}

	bool EditorSceneManager::AddScene(const std::string& sceneName, EMapType type)
	{
		if (m_mapScenes.contains(sceneName))
		{
			F_ERROR("Failed to add new scene object: '{}' already exists", sceneName);
			return false;
		}

		auto [itr, isSuccess] = m_mapScenes.emplace(sceneName, std::move(std::make_shared<SceneObject>(sceneName, type)));

		return isSuccess;
	}

	bool EditorSceneManager::AddSceneObject(const std::string& sceneName, const std::string& sceneData)
	{
		if (m_mapScenes.contains(sceneName))
		{
			F_ERROR("Failed to add new scene object - '{}' already exists", sceneName);
			return false;
		}

		auto [itr, bSuccess] = m_mapScenes.emplace(sceneName, std::move(std::make_shared<SceneObject>(sceneName, sceneData)));
		return bSuccess;
	}

	bool EditorSceneManager::DeleteScene(const std::string& sceneName)
	{
		auto sceneItr = m_mapScenes.find(sceneName);
		if (sceneItr == m_mapScenes.end())
		{
			F_ERROR("Failed to delete scene object: '{}' does not exist in the scene manager", sceneName);
			return false;
		}

		// Check to see if the scene is loaded. We do not want to delete a loaded scene.
		if (sceneItr->second->IsLoaded())
		{
			F_ERROR("Failed to delete scene '{}': Loaded scenes cannot be deleted. Please unload the scene if you want to delete it", sceneName);
			return false;
		}

		const std::string& sDataPath{ sceneItr->second->GetSceneDataPath() };
		std::filesystem::path dataPath{ sDataPath };

		if (std::filesystem::exists(dataPath.parent_path()) && std::filesystem::is_directory(dataPath.parent_path()))
		{
			if (dataPath.parent_path().stem().string() == sceneName)
			{
				std::error_code ec;
				if (!std::filesystem::remove_all(dataPath.parent_path(), ec))
				{
					F_ERROR("Failed to delete scene '{}' and remove files", sceneName, ec.message());
					return false;
				}
			}
		}

		// Recheck if the path exists
		if (std::filesystem::exists(dataPath.parent_path()))
		{
			F_ERROR("Failed to delete scene '{}' and remove files", sceneName);
			return false;
		}

		if (m_mapScenes.erase(sceneName) > 0)
		{
			auto& pSaveProject = MAIN_REGISTRY().GetContext<std::shared_ptr<SaveProject>>();
			F_ASSERT(pSaveProject && "Save Project must exist!");
			// Save entire project
			ProjectLoader pl{};
			if (!pl.SaveLoadedProject(*pSaveProject))
			{
				F_ERROR("Failed to save project '{}' at file '{}' after deleting scene. Please ensure the scene files have been removed",
					pSaveProject->projectName,
					pSaveProject->projectFilePath);

				return false;
			}
		}

		return true;
	}

	ToolManager& EditorSceneManager::GetToolManager()
	{
		if (!m_ToolManager)
			m_ToolManager = std::make_unique<ToolManager>();

		F_ASSERT(m_ToolManager && "Tool manager must be valid!");
		return *m_ToolManager;
	}

	CommandManager& EditorSceneManager::GetCommandManager()
	{
		if (!m_CommandManager)
			m_CommandManager = std::make_unique<CommandManager>();

		F_ASSERT(m_CommandManager && "Command manager must be valid!");
		return *m_CommandManager;
	}

	EventDispatcher& EditorSceneManager::GetDispatcher()
	{
		if (!m_SceneDispatcher)
			m_SceneDispatcher = std::make_unique<EventDispatcher>();

		F_ASSERT(m_SceneDispatcher && "Event Dispatcher must be valid!");
		return *m_SceneDispatcher;
	}

	void EditorSceneManager::SetTileset(const std::string& tileset)
	{
		m_CurrentTileset = tileset;
		if (!m_ToolManager)
			return;

		m_ToolManager->SetToolsCurrentTileset(tileset);
	}

	SceneObject* EditorSceneManager::GetCurrentSceneObject()
	{
		if (auto pCurrentScene = dynamic_cast<SceneObject*>(GetCurrentScene()))
			return pCurrentScene;

		return nullptr;
	}

	bool EditorSceneManager::SaveAllScenes()
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

	bool EditorSceneManager::CheckTagName(const std::string& tagName)
	{
		if (auto pScene = GetCurrentScene())
		{
			auto pSceneObject = dynamic_cast<SceneObject*>(pScene);
			return pSceneObject->CheckTagName(tagName);
		}

		return false;
	}

	void EditorSceneManager::UpdateScenes()
	{
		if (auto currentScene = GetCurrentSceneObject())
		{
			UpdateDirtyEntities(currentScene->GetRegistry());
			UpdateDirtyEntities(currentScene->GetRuntimeRegistry());
		}
	}

	EditorSceneManager::EditorSceneManager()
		: SceneManager()
	{}

	void EditorSceneManager::CreateSceneManagerLuaBind(sol::state& lua)
	{
		auto& sceneManager = SCENE_MANAGER();

		lua.new_usertype<EditorSceneManager>(
			"SceneManager",
			sol::no_constructor,
			"changeScene", [&](const std::string& sceneName)
			{
				auto currentScene = sceneManager.GetCurrentSceneObject();
				if (!currentScene)
				{
					F_ERROR("Failed to change to scene '{}': Current scene is invalid", sceneName);
					return false;
				}

				auto* runtimeData = currentScene->GetRuntimeData();
				F_ASSERT(runtimeData && "Runtime Data was not initialized");
				if (runtimeData->sceneName == sceneName)
				{
					F_ERROR("Failed to load scene '{}': Scene has already been loaded", sceneName);
					return false;
				}

				auto scene = sceneManager.GetScene(sceneName);
				if (!scene)
				{
					F_ERROR("Failed to change to scene '{}': Scene '{}' is invalid", sceneName, sceneName);
					return false;
				}

				currentScene->GetRuntimeRegistry().DestroyEntities<ScriptComponent>();
				if (!scene->IsLoaded())
				{
					scene->LoadScene();
				}

				auto sceneObject = dynamic_cast<SceneObject*>(scene);
				F_ASSERT(sceneObject && "Scene must be a valid SceneObject if run in the editor!");
				if (!sceneObject)
				{
					F_ERROR("Failed to load scene '{}': Scene is not a valid SceneObject", sceneName);

					return scene->UnloadScene(false);
				}

				currentScene->CopySceneToRuntime(*sceneObject);

				return scene->UnloadScene(false);
			},
			"getCanvas", [&]
			{
				if (auto currentScene = sceneManager.GetCurrentSceneObject())
				{
					auto* runtimeData = currentScene->GetRuntimeData();
					return runtimeData ? runtimeData->canvas : Canvas{};
				}

				return Canvas{};
			},
			"getDefaultMusic", [&]
			{
				if (auto currentScene = sceneManager.GetCurrentSceneObject())
				{
					auto* runtimeData = currentScene->GetRuntimeData();
					return runtimeData ? runtimeData->defaultMusic : "";
				}

				return std::string{ "" };
			},
			"getCurrentSceneName", [&]
			{
				if (auto currentScene = sceneManager.GetCurrentSceneObject())
				{
					auto* runtimeData = currentScene->GetRuntimeData();
					return runtimeData ? runtimeData->sceneName : "";
				}

				return std::string{ "" };
			}
		);
	}

}
