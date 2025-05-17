#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace Feather {

	enum class EMapType;
	class Scene;

	class SceneManager
	{
	public:
		SceneManager();
		virtual ~SceneManager() {}

		virtual bool AddScene(const std::string& sceneName, EMapType type);
		bool HasScene(const std::string& sceneName);

		Scene* GetScene(const std::string& sceneName);
		Scene* GetCurrentScene();

		std::vector<std::string> GetSceneNames() const;
		bool LoadCurrentScene();
		bool UnloadCurrentScene();
		bool CheckHasScene(const std::string& sceneName);
		bool ChangeSceneName(const std::string& oldName, const std::string& newName);

		inline void SetCurrentScene(const std::string& sceneName) { m_CurrentScene = sceneName; }
		inline const std::string& GetCurrentSceneName() const { return m_CurrentScene; }

	protected:
		std::map<std::string, std::shared_ptr<Scene>> m_mapScenes;
		std::string m_CurrentScene{ "" };
		std::string m_CurrentTileset{ "" };
	};

}
