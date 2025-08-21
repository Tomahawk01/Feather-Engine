#pragma once

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

	struct CreateTileToolRemoveCmd
	{
		Registry* registry{ nullptr };
		std::shared_ptr<Tile> tile{ nullptr };

		void undo();
		void redo();
	};

}
