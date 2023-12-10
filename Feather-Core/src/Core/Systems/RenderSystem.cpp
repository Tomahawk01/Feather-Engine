#include "RenderSystem.h"

#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/Components/SpriteComponent.h"
#include "Core/ECS/Components/TransformComponent.h"
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
		auto& camera = m_Registry.GetContext<std::shared_ptr<Camera2D>>();
		auto& assetManager = m_Registry.GetContext<std::shared_ptr<AssetManager>>();

		auto& spriteShader = assetManager->GetShader("basic");
		auto cam_mat = camera->GetCameraMatrix();

		if (spriteShader.ShaderProgramID() == 0)
		{
			F_ERROR("Sprite shader program has not been set correctly!");
			return;
		}

		// Enable shader
		spriteShader.Enable();
		spriteShader.SetUniformMat4("u_Projection", cam_mat);

		m_BatchRenderer->Begin();
		auto view = m_Registry.GetRegistry().view<SpriteComponent, TransformComponent>();

		for (const auto& entity : view)
		{
			const auto& transform = view.get<TransformComponent>(entity);
			const auto& sprite = view.get<SpriteComponent>(entity);

			if (sprite.texture_name.empty() || sprite.hidden)
				continue;

			const auto& texture = assetManager->GetTexture(sprite.texture_name);
			if (texture.GetID() == 0)
			{
				F_ERROR("Texture '{0}' was not created correctly!", sprite.texture_name);
				return;
			}

			glm::vec4 spriteRect{ transform.position.x, transform.position.y, sprite.width, sprite.height };
			glm::vec4 uvRect{ sprite.uvs.u, sprite.uvs.v, sprite.uvs.uv_width, sprite.uvs.uv_height };

			glm::mat4 model{ 1.0f };
			if (transform.rotation > 0.0f || transform.rotation < 0.0f ||
				transform.scale.x > 1.0f || transform.scale.x < 1.0f ||
				transform.scale.y > 1.0f || transform.scale.y < 1.0f)
			{
				model = glm::translate(model, glm::vec3{ transform.position, 0.0f });
				model = glm::translate(model, glm::vec3{ (sprite.width * transform.scale.x) * 0.5f, (sprite.height * transform.scale.y) * 0.5f, 0.0f });

				model = glm::rotate(model, glm::radians(transform.rotation), glm::vec3{ 0.0f, 0.0f, 1.0f });
				model = glm::translate(model, glm::vec3{ (sprite.width * transform.scale.x) * -0.5f, (sprite.height * transform.scale.y) * -0.5f, 0.0f });

				model = glm::scale(model, glm::vec3{ transform.scale, 1.0f });

				model = glm::translate(model, glm::vec3{ -transform.position, 0.0f });
			}

			m_BatchRenderer->AddSprite(spriteRect, uvRect, texture.GetID(), sprite.layer, model, sprite.color);
		}

		m_BatchRenderer->End();
		m_BatchRenderer->Render();

		spriteShader.Disable();
	}

}
