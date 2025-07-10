#include "RenderPickingSystem.h"

#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/Components/SpriteComponent.h"
#include "Core/ECS/Components/TransformComponent.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Core/PickingBatchRenderer.h"
#include "Renderer/Essentials/Texture.h"
#include "Renderer/Essentials/Shader.h"

namespace Feather {

	RenderPickingSystem::RenderPickingSystem()
		: m_BatchRenderer{ std::make_unique<PickingBatchRenderer>() }
	{}

	RenderPickingSystem::~RenderPickingSystem()
	{}

	void RenderPickingSystem::Update(Registry& registry, Camera2D& camera)
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		const auto pickingShader = assetManager.GetShader("picking");
		auto cam_mat = camera.GetCameraMatrix();

		if (!pickingShader)
		{
			F_ERROR("Sprite shader program has not been set correctly!");
			return;
		}

		// enable the shader
		pickingShader->Enable();
		pickingShader->SetUniformMat4("uProjection", cam_mat);

		m_BatchRenderer->Begin();
		auto spriteView = registry.GetRegistry().view<SpriteComponent, TransformComponent>(entt::exclude<TileComponent>);
		for (auto entity : spriteView)
		{
			const auto& transform = spriteView.get<TransformComponent>(entity);
			const auto& sprite = spriteView.get<SpriteComponent>(entity);

			if (!EntityInView(transform, sprite.width, sprite.height, camera))
				continue;

			if (sprite.textureName.empty() || sprite.isHidden)
				continue;

			auto texture = assetManager.GetTexture(sprite.textureName);
			if (!texture)
			{
				F_ERROR("Texture '{0}' was not created correctly!", sprite.textureName);
				return;
			}

			glm::vec4 spriteRect{ transform.position.x, transform.position.y, sprite.width, sprite.height };
			glm::vec4 uvRect{ sprite.uvs.u, sprite.uvs.v, sprite.uvs.uv_width, sprite.uvs.uv_height };
			glm::mat4 model = TRSModel(transform, sprite.width, sprite.height);

			m_BatchRenderer->AddSprite(spriteRect, uvRect, texture->GetID(), sprite.layer, static_cast<uint32_t>(entity), sprite.color, model);
		}

		m_BatchRenderer->End();
		m_BatchRenderer->Render();

		pickingShader->Disable();
	}

}
