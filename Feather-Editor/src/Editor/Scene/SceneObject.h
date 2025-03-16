#pragma once
#include "Core/ECS/Entity.h"
#include "Utils/HelperUtilities.h"

#include "Editor/Utilities/EditorUtilities.h"

namespace Feather {

	struct NameChangeEvent;

	class SceneObject
	{
	public:
		SceneObject(const std::string& sceneName);
		SceneObject(const std::string& sceneName, const std::string& sceneData);
		~SceneObject() = default;

		void CopySceneToRuntime();
		void ClearRuntimeScene();

		void AddNewLayer();
		void AddLayer(const std::string& layerName, bool visible);
		bool CheckLayerName(const std::string& layerName);

		bool AddGameObject();
		bool DuplicateGameObject(entt::entity entity);
		bool DeleteGameObjectByTag(const std::string& tag);
		bool DeleteGameObjectById(entt::entity entity);

		bool LoadScene();
		bool UnloadScene();
		bool SaveScene();

		bool CheckTagName(const std::string& tagName);

		inline bool HasTileLayers() const { return !m_LayerParams.empty(); }
		inline std::vector<SpriteLayerParams>& GetLayerParams() { return m_LayerParams; }

		inline Canvas& GetCanvas() { return m_Canvas; }
		inline const std::string& GetSceneName() { return m_SceneName; }
		inline const std::string& GetSceneDataPath() { return m_SceneDataPath; }
		inline Registry& GetRegistry() { return m_Registry; }
		inline Registry* GetRegistryPtr() { return &m_Registry; }
		inline Registry& GetRuntimeRegistry() { return m_RuntimeRegistry; }

	private:
		bool LoadSceneData();
		bool SaveSceneData();
		void OnEntityNameChanges(NameChangeEvent& nameChange);

	private:
		Registry m_Registry;
		Registry m_RuntimeRegistry;

		std::string m_SceneName;
		std::string m_TilemapPath;
		std::string m_ObjectPath;
		std::string m_SceneDataPath;

		Canvas m_Canvas;
		std::vector<SpriteLayerParams> m_LayerParams;
		std::map<std::string, entt::entity> m_mapTagToEntity;
		int m_CurrentLayer;
		bool m_SceneLoaded{ false };
	};

}
