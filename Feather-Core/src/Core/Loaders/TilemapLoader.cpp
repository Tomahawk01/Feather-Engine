#include "TilemapLoader.h"

namespace Feather {

	bool TilemapLoader::SaveTilemap(Registry& registry, const std::string& tilemapFile, bool useJson)
	{
		if (useJson)
			return SaveTilemapJSON(registry, tilemapFile);

		return false;
	}

	bool TilemapLoader::LoadTilemap(Registry& registry, const std::string& tilemapFile, bool useJson)
	{
		if (useJson)
			return LoadTilemapJSON(registry, tilemapFile);

		return false;
	}

	bool TilemapLoader::SaveTilemapJSON(Registry& registry, const std::string& tilemapFile)
	{
		// TODO: 
		return false;
	}

	bool TilemapLoader::LoadTilemapJSON(Registry& registry, const std::string& tilemapFile)
	{
		// TODO: 
		return false;
	}

}
