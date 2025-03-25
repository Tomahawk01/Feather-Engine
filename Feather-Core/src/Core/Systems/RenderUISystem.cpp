#include "RenderUISystem.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Core/Resources/AssetManager.h"
#include "Renderer/Essentials/Font.h"
#include "Renderer/Core/BatchRenderer.h"
#include "Renderer/Core/TextBatchRenderer.h"
#include "Renderer/Core/Camera2D.h"

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

	RenderUISystem::~RenderUISystem()
	{}

	void RenderUISystem::Update(Registry& registry)
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		auto spriteShader = assetManager.GetShader("basic");
		if (!spriteShader)
		{
			F_ERROR("Failed to Render UI, basic shader is invalid");
			return;
		}

		auto& reg = registry.GetRegistry();
		auto spriteView = reg.view<UIComponent, SpriteComponent, TransformComponent>();

		auto cam_mat = m_Camera2D->GetCameraMatrix();
		spriteShader->Enable();
		spriteShader->SetUniformMat4("uProjection", cam_mat);

		m_SpriteRenderer->Begin();

		for (auto entity : spriteView)
		{
			const auto& transform = spriteView.get<TransformComponent>(entity);
			const auto& sprite = spriteView.get<SpriteComponent>(entity);

			if (sprite.textureName.empty() || sprite.isHidden)
				continue;

			const auto& pTexture = assetManager.GetTexture(sprite.textureName);
			if (!pTexture)
			{
				F_ERROR("Texture '{0}' was not created correctly!", sprite.textureName);
				return;
			}

			glm::vec4 spriteRect{ transform.position.x, transform.position.y, sprite.width, sprite.height };
			glm::vec4 uvRect{ sprite.uvs.u, sprite.uvs.v, sprite.uvs.uv_width, sprite.uvs.uv_height };

			glm::mat4 model = TRSModel(transform, sprite.width, sprite.height);

			m_SpriteRenderer->AddSprite(spriteRect, uvRect, pTexture->GetID(), sprite.layer, model, sprite.color);
		}

		m_SpriteRenderer->End();
		m_SpriteRenderer->Render();

		spriteShader->Disable();

		// If there are no entities in the view, leave
		auto textView = reg.view<TextComponent, TransformComponent>();
		if (textView.size_hint() < 1)
			return;

		auto fontShader = assetManager.GetShader("font");

		if (!fontShader)
		{
			F_ERROR("Failed to get the font shader from the asset manager!");
			return;
		}

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

	void RenderUISystem::CreateRenderUISystemLuaBind(sol::state& lua)
	{
		lua.new_usertype<RenderUISystem>(
			"RenderUISystem",
			sol::call_constructor,
			sol::constructors<RenderUISystem()>(),
			"update",
			[&](RenderUISystem& system, Registry& reg) { system.Update(reg); }
		);
	}

}
