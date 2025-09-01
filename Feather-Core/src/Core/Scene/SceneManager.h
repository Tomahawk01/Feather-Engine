#pragma once

#include <sol/sol.hpp>

namespace Feather {

	enum class EMapType;
	class Scene;
	class Registry;

	struct SceneManagerData
	{
		std::string sceneName{};
		std::string defaultMusic{};
		// TODO: Add more stuff
	};

	class SceneManager
	{
	public:
		SceneManager();
		virtual ~SceneManager() {}

		/**
		* @brief Adds a new scene to the scene manager with the given name and map type.
		*
		* Creates and stores a new Scene object if the name is not already in use. Logs an error and returns false
		* if a scene with the same name already exists.
		*
		* @param sceneName The name of the scene to add.
		* @param type The type of the map associated with the scene.
		* @return true if the scene was successfully added; false if the name already exists.
		*/
		virtual bool AddScene(const std::string& sceneName, EMapType type);
		bool HasScene(const std::string& sceneName);

		/**
		* @brief Checks whether a the scene exists and returns a pointer to that scene.
		*
		* Determines if the given scene name is present in the sceneName-to-scene map.
		*
		* @param sceneName The scene name to check.
		* @return a pointer to the scene if exists, nullptr otherwise.
		*/
		Scene* GetScene(const std::string& sceneName);

		/**
		* @brief Returns a pointer to the current selected scene.
		*
		* @return pointer to the current scene if set, nullptr otherwise.
		*/
		Scene* GetCurrentScene();

		std::vector<std::string> GetSceneNames() const;
		bool LoadCurrentScene();
		bool UnloadCurrentScene();
		bool CheckHasScene(const std::string& sceneName);
		bool ChangeSceneName(const std::string& oldName, const std::string& newName);

		inline void SetCurrentScene(const std::string& sceneName) { m_CurrentScene = sceneName; }
		inline const std::string& GetCurrentSceneName() const { return m_CurrentScene; }

		static void CreateLuaBind(sol::state& lua, Registry& registry);

	protected:
		std::map<std::string, std::shared_ptr<Scene>> m_mapScenes;
		std::string m_CurrentScene{};
		std::string m_CurrentTileset{};
	};

}
