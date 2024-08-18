#include "CreateTileTool.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/MainRegistry.h"
#include "Renderer/Core/Camera2D.h"
#include "Logger/Logger.h"
#include "../Utilities/EditorUtilities.h"

namespace Feather {

	CreateTileTool::CreateTileTool()
		: TileTool()
	{}

	void CreateTileTool::Create()
	{
		// TODO: Create tile entity based of the mouse tile
	}

	void CreateTileTool::Draw()
	{
		if (!SpriteValid() || !IsActivated() || OutOfBounds() || !m_Camera)
			return;

		auto shader = MAIN_REGISTRY().GetAssetManager().GetShader("basic");
		if (!shader)
			return;

		shader->Enable();
		auto camMat = m_Camera->GetCameraMatrix();
		shader->SetUniformMat4("u_Projection", camMat);
		DrawMouseSprite();
		shader->Disable();
	}

}
