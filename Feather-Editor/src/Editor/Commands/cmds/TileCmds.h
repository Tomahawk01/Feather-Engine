#pragma once

#include "Utils/HelperUtilities.h"

namespace Feather {

	struct Tile;
	class Registry;
	class SceneObject;

	struct RemoveTileLayerCmd
	{
		SceneObject* sceneObject{ nullptr };
		std::vector<Tile> tilesRemoved;
		SpriteLayerParams spriteLayerParams;

		void undo();
		void redo();
	};

	struct AddTileLayerCmd
	{
		SceneObject* sceneObject{ nullptr };
		SpriteLayerParams spriteLayerParams;

		void undo();
		void redo();
	};

	struct MoveTileLayerCmd
	{
		SceneObject* sceneObject{ nullptr };
		int from;
		int to;

		void undo();
		void redo();
	};

	struct ChangeTileLayerNameCmd
	{
		SceneObject* sceneObject{ nullptr };
		std::string oldName{};
		std::string newName{};

		void undo();
		void redo();
	};

}
