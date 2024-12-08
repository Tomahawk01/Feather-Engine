#pragma once

#include <vector>

namespace Feather {

	struct Tile;
	class Registry;

	struct RectToolAddTilesCmd
	{
		Registry* registry{ nullptr };
		std::vector<Tile> tiles{};

		void undo();
		void redo();
	};

	struct RectToolRemoveTilesCmd
	{
		Registry* registry{ nullptr };
		std::vector<Tile> tiles{};

		void undo();
		void redo();
	};

}
