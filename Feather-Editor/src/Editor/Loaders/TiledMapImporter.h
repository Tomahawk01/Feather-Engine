#pragma once

#include <glm/glm.hpp>

namespace Feather {

	class EditorSceneManager;

	class TiledMapImporter
	{
	public:
		TiledMapImporter() = delete;

		/* @brief Imports tilemaps made in the Tiled Map Editor and converts them to a new Feather Map and Scene.
		* @param sceneManager a pointer to the scene manager to create a new scene.
		* @param tiledMapFile the path to the map file to convert.
		* @return true if successful, false otherwise.
		*/
		static bool ImportTilemapFromTiled(EditorSceneManager* sceneManager, const std::string tiledMapFile);

	private:
		struct TileObject
		{
			std::string name{};
			std::string type{};
			glm::vec2 position{ 0.0f };
			float width{ 0.0f };
			float height{ 0.0f };
			float rotation{ 0.0f };
		};

		struct Tile
		{
			int id{ -1 };
			std::vector<TileObject> tileObjects;
		};

		struct Tileset
		{
			std::string name{};
			int columns{ 1 };
			int rows{ 1 };
			int width{ 1 };
			int height{ 1 };
			int tileWidth{ 16 };
			int tileHeight{ 16 };
			int firstGID{ 1 };
			int tileCount{ 1 };
			std::vector<Tile> tiles{};

			inline void SetRows() { rows = height / tileHeight; }
			inline const bool TileIdExists(int id) const { return id >= firstGID && id <= tileCount; }
			std::tuple<int, int> GetTileStartXY(int id);
			TileObject* GetObjectFromId(int id);
		};
	};

}
