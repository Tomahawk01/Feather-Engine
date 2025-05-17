#pragma once
#include "IDisplay.h"

#include <string>

namespace Feather {

	struct SpriteComponent;
	class Scene;

	class TileDetailsDisplay : public IDisplay
	{
	public:
		TileDetailsDisplay();
		virtual ~TileDetailsDisplay();

		virtual void Draw() override;

	private:
		void DrawSpriteComponent(SpriteComponent& sprite, Scene* scene);

	private:
		int m_SelectedLayer;
		std::string m_RenameLayerBuff;
		bool m_Renaming;
	};

}
