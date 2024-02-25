#include "RenderUISystem.h"

#include "Logger/Logger.h"
#include "Core/ECS/Components/TextComponent.h"
#include "Core/ECS/Components/TransformComponent.h"
#include "Core/Resources/AssetManager.h"
#include "Renderer/Essentials/Font.h"

namespace Feather {

	RenderUISystem::RenderUISystem(Registry& registry)
		: m_Registry{ registry },
		m_SpriteRenderer{ std::make_unique<SpriteBatchRenderer>() },
		m_TextRenderer{ std::make_unique<TextBatchRenderer>() },
		m_Camera2D{ nullptr }
	{
		m_Camera2D = std::make_unique<Camera2D>(640, 480); // TODO: Change based on window values
		m_Camera2D->Update();
	}

	void RenderUISystem::Update(entt::registry& registry)
	{
		auto& assetManager = m_Registry.GetContext<std::shared_ptr<AssetManager>>();
		auto fontShader = assetManager->GetShader("font");

		if (!fontShader)
		{
			F_ERROR("Failed to get the font shader from the asset manager!");
			return;
		}

		auto cam_mat = m_Camera2D->GetCameraMatrix();
		auto textView = registry.view<TextComponent, TransformComponent>();

		fontShader->Enable();
		fontShader->SetUniformMat4("uProjection", cam_mat);

		m_TextRenderer->Begin();

		for (auto entity : textView)
		{
			const auto& text = textView.get<TextComponent>(entity);
			if (text.fontName.empty() || text.isHidden)
				continue;

			const auto& font = assetManager->GetFont(text.fontName);
			if (!font)
			{
				F_ERROR("Font '{}' does not exist in the asset manager!", text.fontName);
				continue;
			}

			const auto& transform = textView.get<TransformComponent>(entity);

			glm::mat4 model{ 1.0f };
			if (transform.rotation > 0.0f || transform.rotation < 0.0f ||
				transform.scale.x > 1.0f || transform.scale.x < 1.0f ||
				transform.scale.y > 1.0f || transform.scale.y < 1.0f)
			{
				model = glm::translate(model, glm::vec3{ transform.position, 0.0f });
				model = glm::translate(model, glm::vec3{ (font->GetFontSize() * transform.scale.x) * 0.5f, (font->GetFontSize() * transform.scale.y) * 0.5f, 0.0f });

				model = glm::rotate(model, glm::radians(transform.rotation), glm::vec3{ 0.0f, 0.0f, 1.0f });
				model = glm::translate(model, glm::vec3{ (font->GetFontSize() * transform.scale.x) * -0.5f, (font->GetFontSize() * transform.scale.y) * -0.5f, 0.0f });

				model = glm::scale(model, glm::vec3{ transform.scale, 1.0f });

				model = glm::translate(model, glm::vec3{ -transform.position, 0.0f });
			}

			m_TextRenderer->AddText(text.textStr, font, transform.position, text.padding, text.wrap, text.color, model);
		}

		m_TextRenderer->End();
		m_TextRenderer->Render();

		fontShader->Disable();
	}

}
