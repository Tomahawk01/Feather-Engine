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

		bool LoadGameObjects(Registry& registry, const std::string& objectMapFile, bool useJSON = false);
		bool SaveGameObjects(Registry& registry, const std::string& objectMapFile, bool useJSON = false);

	private:
		bool SaveTilemapJSON(Registry& registry, const std::string& tilemapFile);
		bool LoadTilemapJSON(Registry& registry, const std::string& tilemapFile);

		bool SaveObjectMapJSON(Registry& registry, const std::string& objectMapFile);
		bool LoadObjectMapJSON(Registry& registry, const std::string& objectMapFile);

		// TODO: Save and load functions for Lua serializer
	};

	struct SaveRelationship
	{
		//entt::entity Self{ entt::null };
		std::string Parent{ "" };
		std::string NextSibling{ "" };
		std::string PrevSibling{ "" };
		std::string FirstChild{ "" };
	};

}
