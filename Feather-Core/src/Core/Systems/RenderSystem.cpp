#include "RenderSystem.h"

#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Essentials/Shader.h"

namespace Feather {

	RenderSystem::RenderSystem(Registry& registry)
		: m_Registry(registry), m_BatchRenderer{nullptr}
	{
		m_BatchRenderer = std::make_unique<SpriteBatchRenderer>();
	}

	void RenderSystem::Update()
	{
		auto view = m_Registry.GetRegistry().view<SpriteComponent, TransformComponent>();
		if (view.size_hint() < 1)
			return;

		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		auto& camera = m_Registry.GetContext<std::shared_ptr<Camera2D>>();

		const auto& spriteShader = assetManager.GetShader("basic");
		auto cam_mat = camera->GetCameraMatrix();

		if (spriteShader->ShaderProgramID() == 0)
		{
			F_ERROR("Sprite shader program has not been set correctly!");
			return;
		}

		// Enable shader
		spriteShader->Enable();
		spriteShader->SetUniformMat4("u_Projection", cam_mat);

		m_BatchRenderer->Begin();

		for (const auto& entity : view)
		{
			const auto& transform = view.get<TransformComponent>(entity);
			const auto& sprite = view.get<SpriteComponent>(entity);

			if (!EntityInView(transform, sprite.width, sprite.height, *camera))
				continue;

			if (sprite.texture_name.empty() || sprite.isHidden)
				continue;

			const auto& pTexture = assetManager.GetTexture(sprite.texture_name);
			if (!pTexture)
			{
				F_ERROR("Texture '{0}' was not created correctly!", sprite.texture_name);
				return;
			}

			glm::vec4 spriteRect{ transform.position.x, transform.position.y, sprite.width, sprite.height };
			glm::vec4 uvRect{ sprite.uvs.u, sprite.uvs.v, sprite.uvs.uv_width, sprite.uvs.uv_height };

			glm::mat4 model = TRSModel(transform, sprite.width, sprite.height);

			m_BatchRenderer->AddSprite(spriteRect, uvRect, pTexture->GetID(), sprite.layer, model, sprite.color);
		}

		m_BatchRenderer->End();
		m_BatchRenderer->Render();

		spriteShader->Disable();
	}

}
