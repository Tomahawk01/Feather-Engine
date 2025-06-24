#pragma once

#include "Core/Scene/Scene.h"

#include "Editor/Utilities/EditorUtilities.h"

namespace Feather {

	struct NameChangeEvent;

	class SceneObject : public Scene
	{
	public:
		SceneObject(const std::string& sceneName, EMapType eType = EMapType::Grid);
		SceneObject(const std::string& sceneName, const std::string& sceneData);
		~SceneObject() = default;

		void CopySceneToRuntime();
		void CopySceneToRuntime(SceneObject& sceneToCopy);
		void CopyPlayerStartToRuntimeRegistry(Registry& runtimeRegistry);
		void ClearRuntimeScene();

		void AddNewLayer();
		
		bool AddGameObject();
		bool AddGameObjectByTag(const std::string& tag, entt::entity entity);
		bool DuplicateGameObject(entt::entity entity);
		bool DeleteGameObjectByTag(const std::string& tag);
		bool DeleteGameObjectById(entt::entity entity);

		virtual bool LoadScene() override;
		virtual bool UnloadScene(bool saveScene = true) override;

		bool CheckTagName(const std::string& tagName);

		inline const std::string& GetSceneName() { return m_SceneName; }
		inline const std::string& GetRuntimeName() { return m_RuntimeSceneName; }
		inline Registry& GetRuntimeRegistry() { return m_RuntimeRegistry; }

	private:
		void OnEntityNameChanges(NameChangeEvent& nameChange);

	private:
		Registry m_RuntimeRegistry;

		std::string m_RuntimeSceneName;

		std::map<std::string, entt::entity> m_mapTagToEntity;
		int m_CurrentLayer;
	};

}
