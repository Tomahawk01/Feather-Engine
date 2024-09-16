#pragma once
#include <string>

namespace Feather {

	class Registry;

	class TilemapLoader
	{
	public:
		TilemapLoader() = default;
		~TilemapLoader() = default;

		bool SaveTilemap(Registry& registry, const std::string& tilemapFile, bool useJson = false);
		bool LoadTilemap(Registry& registry, const std::string& tilemapFile, bool useJson = false);

	private:
		bool SaveTilemapJSON(Registry& registry, const std::string& tilemapFile);
		bool LoadTilemapJSON(Registry& registry, const std::string& tilemapFile);

		// TODO: Save and load functions for Lua serializer
	};

}
