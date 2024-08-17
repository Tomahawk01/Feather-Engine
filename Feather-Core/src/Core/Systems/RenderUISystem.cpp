#include "RenderUISystem.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Core/Resources/AssetManager.h"
#include "Renderer/Essentials/Font.h"

namespace Feather {

	RenderUISystem::RenderUISystem()
		: m_SpriteRenderer{ std::make_unique<SpriteBatchRenderer>() },
		m_TextRenderer{ std::make_unique<TextBatchRenderer>() },
		m_Camera2D{ nullptr }
	{
		auto& coreEngine = CoreEngineData::GetInstance();

		m_Camera2D = std::make_unique<Camera2D>(
			coreEngine.WindowWidth(),
			coreEngine.WindowHeight()
		);
		m_Camera2D->Update();
	}

	void RenderUISystem::Update(Registry& registry)
	{
		auto& mainRegistry = MAIN_REGISTRY();

		// If there are no entities in the view, leave
		auto textView = registry.GetRegistry().view<TextComponent, TransformComponent>();
		if (textView.size_hint() < 1)
			return;

		auto& assetManager = mainRegistry.GetAssetManager();
		auto fontShader = assetManager.GetShader("font");

		if (!fontShader)
		{
			F_ERROR("Failed to get the font shader from the asset manager!");
			return;
		}

		auto cam_mat = m_Camera2D->GetCameraMatrix();

		fontShader->Enable();
		fontShader->SetUniformMat4("uProjection", cam_mat);

		m_TextRenderer->Begin();

		for (auto entity : textView)
		{
			const auto& text = textView.get<TextComponent>(entity);
			if (text.fontName.empty() || text.isHidden)
				continue;

			const auto& font = assetManager.GetFont(text.fontName);
			if (!font)
			{
				F_ERROR("Font '{}' does not exist in the asset manager!", text.fontName);
				continue;
			}

			const auto& transform = textView.get<TransformComponent>(entity);
			const auto fontSize = font->GetFontSize();

			glm::mat4 model = TRSModel(transform, fontSize, fontSize);

			m_TextRenderer->AddText(text.textStr, font, transform.position, text.padding, text.wrap, text.color, model);
		}

		m_TextRenderer->End();
		m_TextRenderer->Render();

		fontShader->Disable();
	}

}
