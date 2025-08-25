#pragma once

#include "IDisplay.h"

namespace Feather {

	class TilesetDisplay : public IDisplay
	{
	public:
		TilesetDisplay() = default;
		~TilesetDisplay() = default;

		virtual void Draw() override;

		inline void SetTileset(const std::string& tileset) { m_Tileset = tileset; }
		inline const std::string& GetTilesetName() const { return m_Tileset; }

	protected:
		virtual void DrawToolbar() override;

	private:
		std::string m_Tileset{};

		struct TableSelection
		{
			int startRow{ -1 };
			int startCol{ -1 };
			int endRow{ -1 };
			int endCol{ -1 };
			bool selecting{ false };

			bool IsValid() const { return startRow != -1 && startCol != -1; }

			void Reset()
			{
				startRow = startCol = endRow = endCol = -1;
				selecting = false;
			}
		};

		TableSelection m_Selection;
	};

}
