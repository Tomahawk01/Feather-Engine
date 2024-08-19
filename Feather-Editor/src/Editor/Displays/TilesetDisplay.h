#pragma once
#include "IDisplay.h"

#include <string>

namespace Feather {

	class TilesetDisplay : public IDisplay
	{
	public:
		TilesetDisplay() = default;
		virtual ~TilesetDisplay();

		virtual void Draw() override;

		inline void SetTileset(const std::string& tileset) { m_Tileset = tileset; }
		inline const std::string& GetTilesetName() const { return m_Tileset; }

	private:
		std::string m_Tileset{ "" };
		int m_Selected{ -1 };
	};

}
