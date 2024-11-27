#pragma once
#include <memory>

namespace Feather {

	class Registry;
	struct Tile;

	struct CreateTileToolAddCmd
	{
		Registry* registry{ nullptr };
		std::shared_ptr<Tile> tile{ nullptr };

		void undo();
		void redo();
	};

}
