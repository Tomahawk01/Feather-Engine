#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#define SCENE_MANAGER() Feather::SceneManager::GetInstance()
#define COMMAND_MANAGER() SCENE_MANAGER().GetCommandManager()
#define TOOL_MANAGER() SCENE_MANAGER().GetToolManager()

namespace Feather {

	class ToolManager;
	class SceneObject;
	class CommandManager;
	class EventDispatcher;

	class SceneManager
	{
	public:
		static SceneManager& GetInstance();

		bool AddScene(const std::string& sceneName);
		bool AddScene(const std::string& sceneName, const std::string& sceneData);
		bool HasScene(const std::string& sceneName);

		std::shared_ptr<SceneObject> GetScene(const std::string& sceneName);
		std::shared_ptr<SceneObject> GetCurrentScene();

		std::vector<std::string> GetSceneNames() const;
		ToolManager& GetToolManager();
		CommandManager& GetCommandManager();
		EventDispatcher& GetDispatcher();

		void SetTileset(const std::string& tileset);

		bool LoadCurrentScene();
		bool UnloadCurrentScene();

		bool SaveAllScenes();

		bool CheckTagName(const std::string& tagName);

		inline const std::map<std::string, std::shared_ptr<SceneObject>>& GetAllScenes() const { return m_mapScenes; }
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
		std::string m_CurrentScene{ "" };
		std::string m_CurrentTileset{ "" };

		std::unique_ptr<ToolManager> m_ToolManager{ nullptr };
		std::unique_ptr<CommandManager> m_CommandManager{ nullptr };

		std::unique_ptr<EventDispatcher> m_SceneDispatcher{ nullptr };
	};

}

#define ADD_SWE_HANDLER(Event, Func, Handler)							\
	{																	\
		for (auto& dispatcher : TOOL_MANAGER().GetDispatchers())		\
		{																\
			if (!dispatcher)											\
				continue;												\
			dispatcher->AddHandler<Event, Func>(Handler);				\
		}																\
	}
