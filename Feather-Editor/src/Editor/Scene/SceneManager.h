#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#define SCENE_MANAGER() Feather::SceneManager::GetInstance()

namespace Feather {

	class SceneObject;

	class SceneManager
	{
	public:
		static SceneManager& GetInstance();

		bool AddScene(const std::string& sceneName);
		std::shared_ptr<SceneObject> GetScene(const std::string& sceneName);
		std::shared_ptr<SceneObject> GetCurrentScene();
		std::vector<std::string> GetSceneNames() const;

		inline void SetCurrentScene(const std::string& sceneName) { m_CurrentScene = sceneName; }
		inline const std::string& GetCurrentSceneName() const { return m_CurrentScene; }

	private:
		SceneManager() = default;
		~SceneManager() = default;
		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;

	private:
		std::map<std::string, std::shared_ptr<SceneObject>> m_mapScenes;
		std::string m_CurrentScene{ "" };
	};

}
