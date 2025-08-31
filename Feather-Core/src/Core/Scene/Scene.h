#pragma once

#include "Core/ECS/Entity.h"
#include "Core/Character/PlayerStart.h"
#include "Utils/HelperUtilities.h"

#include <glm/glm.hpp>
#include <sol/sol.hpp>

namespace Feather {

	struct Canvas
	{
		int width{ 640 };
		int height{ 480 };
		int tileWidth{ 16 };
		int tileHeight{ 16 };
		glm::vec2 offset{ 0.0f };
	};

	enum class EMapType
	{
		Grid,
		IsoGrid
	};

	class Scene
	{
	public:
		Scene();
		Scene(const std::string& sceneName, EMapType type);
		virtual ~Scene() {};

		/*
		 * @brief Tries to load the scene. Loads the tilemap, layers,
		 * game objects, and other scene data.
		 * @return true if successful, false otherwise.
		 */
		virtual bool LoadScene();

		/*
		 * @brief Tries to Unload the scene. When unloading, this will
		 * try to save the tilemap, layers, game objects, and other scene data.
		 * The scene's registry will also be cleared.
		 * @return true if successful, false otherwise.
		 */
		virtual bool UnloadScene(bool saveScene = true);

		/*
		 * @brief Tries to add a specific layer and set it's visibility.
		 * If the layer already exists, it will do nothing.
		 * @param string for the layer name.
		 * @param bool to set the visibility of the layer.
		 */
		int AddLayer(const std::string& layerName, bool visible);

		int AddLayer(const SpriteLayerParams& layerParam);

		/*
		 * @brief Checks to see if the layer already exists in the scene.
		 * To be used when trying to adjust the name of the layer to prevent layers with duplicate names.
		 * @param Takes in a string for the layer to check.
		 * @return true if that layer does not exist, false otherwise.
		 */
		bool CheckLayerName(const std::string& layerName);

		/*
		 * @brief Tries to save the scene. This differs from the unload function
		 * because it does not set the loaded flag or clear the registry.
		 * @return true if successful, false otherwise.
		 */
		bool SaveScene(bool bOverride = false) { return SaveSceneData(bOverride); }

		inline const std::string& GetDefaultMusicName() const { return m_DefaultMusic; }
		inline void SetDefaultMusic(const std::string& defaultMusic) { m_DefaultMusic = defaultMusic; }

		inline Canvas& GetCanvas() { return m_Canvas; }
		inline Registry& GetRegistry() { return m_Registry; }
		inline Registry* GetRegistryPtr() { return &m_Registry; }
		inline const std::string& GetSceneName() const { return m_SceneName; }
		inline const std::string& GetSceneDataPath() { return m_SceneDataPath; }
		inline EMapType GetMapType() const { return m_MapType; }
		inline const std::string& GetFilepath() const { return m_SceneDataPath; }
		inline bool IsLoaded() const { return m_SceneLoaded; }
		/*
		 * @brief Checks to see if there are any layers in the scenes tilemap.
		 * @return true if there are layers present, false otherwise.
		 */
		inline bool HasTileLayers() const { return !m_LayerParams.empty(); }
		inline std::vector<SpriteLayerParams>& GetLayerParams() { return m_LayerParams; }

		inline PlayerStart& GetPlayerStart() { return m_PlayerStart; }
		inline bool IsPlayerStartEnabled() const { return m_UsePlayerStart; }
		inline void SetPlayerStartEnabled(bool enable) { m_UsePlayerStart = enable; }

		static void CreateLuaBind(sol::state& lua);

	protected:
		bool LoadSceneData();
		bool SaveSceneData(bool override = false);
		void SetCanvasOffset();

	protected:
		/* The registry that is used in the tilemap editor and the scene hierarchy */
		Registry m_Registry;
		/* The name of the scene object represents */
		std::string m_SceneName;
		/* The filepath which to load and save the tilemap */
		std::string m_TilemapPath;
		/* The filepath which to load and save the game objects */
		std::string m_ObjectPath;
		/* The filepath which to load and save the scene data */
		std::string m_SceneDataPath;
		/* The default music to play for the scene */
		std::string m_DefaultMusic;
		/* Has this scene been loaded in the editor */
		bool m_SceneLoaded;
		/* All scenes will have a default player start. This flag will control it's use */
		bool m_UsePlayerStart;

		Canvas m_Canvas;

		/* The type of map to create tiles. Iso or regular grid */
		EMapType m_MapType;

		std::vector<SpriteLayerParams> m_LayerParams;

		PlayerStart m_PlayerStart;
	};

}
