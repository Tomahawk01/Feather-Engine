#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#define SCENE_MANAGER() Feather::SceneManager::GetInstance()
#define COMMAND_MANAGER() SCENE_MANAGER().GetCommandManager()

namespace Feather {

	class ToolManager;
	class SceneObject;
	class CommandManager;

	class SceneManager
	{
	public:
		static SceneManager& GetInstance();

		bool AddScene(const std::string& sceneName);
		std::shared_ptr<SceneObject> GetScene(const std::string& sceneName);
		std::shared_ptr<SceneObject> GetCurrentScene();
		std::vector<std::string> GetSceneNames() const;
		ToolManager& GetToolManager();
		CommandManager& GetCommandManager();

		void SetTileset(const std::string& tileset);

		inline void SetCurrentScene(const std::string& sceneName) { m_CurrentScene = sceneName; }
		inline const std::string& GetCurrentSceneName() const { return m_CurrentScene; }
		inline const std::string& GetCurrentTileset() const { return m_CurrentTileset; }

	private:
		SceneManager() = default;
		~SceneManager() = default;
		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;

	private:
		std::map<std::string, std::shared_ptr<SceneObject>> m_mapScenes;
		std::string m_CurrentScene{ "" }, m_CurrentTileset{ "" };

		std::unique_ptr<ToolManager> m_ToolManager{ nullptr };
		std::unique_ptr<CommandManager> m_CommandManager{ nullptr };
	};

}
