#pragma once

#include "Core/Scene/SceneManager.h"

#include <sol/sol.hpp>

#define SCENE_MANAGER() Feather::EditorSceneManager::GetInstance()
#define COMMAND_MANAGER() SCENE_MANAGER().GetCommandManager()
#define TOOL_MANAGER() SCENE_MANAGER().GetToolManager()

namespace Feather {

	enum class EMapType;
	class ToolManager;
	class SceneObject;
	class CommandManager;
	class EventDispatcher;

	class EditorSceneManager : public SceneManager
	{
	public:
		static EditorSceneManager& GetInstance();

		virtual bool AddScene(const std::string& sceneName, EMapType type) override;
		bool AddSceneObject(const std::string& sceneName, const std::string& sceneData);
		bool DeleteScene(const std::string& sceneName);

		ToolManager& GetToolManager();
		CommandManager& GetCommandManager();
		EventDispatcher& GetDispatcher();

		void SetTileset(const std::string& tileset);

		SceneObject* GetCurrentSceneObject();

		bool SaveAllScenes();

		bool CheckTagName(const std::string& tagName);

		void UpdateScenes();

		inline const std::map<std::string, std::shared_ptr<Scene>>& GetAllScenes() const { return m_mapScenes; }
		inline const std::string& GetCurrentTileset() const { return m_CurrentTileset; }

		static void CreateSceneManagerLuaBind(sol::state& lua);

	private:
		EditorSceneManager();
		virtual ~EditorSceneManager() = default;
		EditorSceneManager(const EditorSceneManager&) = delete;
		EditorSceneManager& operator=(const EditorSceneManager&) = delete;

	private:
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
