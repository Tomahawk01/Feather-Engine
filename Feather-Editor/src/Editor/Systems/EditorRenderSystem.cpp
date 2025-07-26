#include "EditorRenderSystem.h"

#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Essentials/Shader.h"
#include "Renderer/Essentials/Texture.h"
#include "Renderer/Core/BatchRenderer.h"
#include "Utils/HelperUtilities.h"

#include <ranges>

namespace Feather {

	EditorRenderSystem::EditorRenderSystem()
		: m_BatchRenderer{ std::make_unique<SpriteBatchRenderer>() }
	{}

	EditorRenderSystem::~EditorRenderSystem() = default;

	void EditorRenderSystem::Update(Registry& registry, Camera2D& camera, const std::vector<SpriteLayerParams>& layerFilters)
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		auto spriteShader = assetManager.GetShader("basic");
		auto cam_mat = camera.GetCameraMatrix();

		if (spriteShader->ShaderProgramID() == 0)
		{
			F_ERROR("Sprite shader program has not been set correctly!");
			return;
		}

		// enable the shader
		spriteShader->Enable();
		spriteShader->SetUniformMat4("uProjection", cam_mat);

		m_BatchRenderer->Begin();

		auto spriteView = registry.GetRegistry().view<SpriteComponent, TransformComponent>(entt::exclude<UIComponent>);
		std::function<bool(entt::entity)> filterFunc;

		// Check to see if the layers are visible, if not, filter them out
		if (layerFilters.empty())
		{
			filterFunc = [](entt::entity) { return true; };
		}
		else
		{
			filterFunc = [&](entt::entity entity)
			{
				// We only want to filter tiles
				if (!registry.GetRegistry().all_of<TileComponent>(entity))
					return true;

				const auto& sprite = spriteView.get<SpriteComponent>(entity);
				if (sprite.layer >= 0)
				{
					auto layerItr = std::ranges::find_if(layerFilters,
														 [&sprite](const auto& layerParams)
														 {
															 return layerParams.layer == sprite.layer;
														 }
					);

					return layerItr != layerFilters.end() ? layerItr->isVisible : false;
				}

				return false;
			};
		}

		for (const auto& entity : std::views::filter(spriteView, filterFunc))
		{
			const auto& transform = spriteView.get<TransformComponent>(entity);
			const auto& sprite = spriteView.get<SpriteComponent>(entity);

			if (!EntityInView(transform, sprite.width, sprite.height, camera))
				continue;

			if (sprite.textureName.empty() || sprite.isHidden)
				continue;

			const auto& pTexture = assetManager.GetTexture(sprite.textureName);
			if (!pTexture)
			{
				F_ERROR("Texture '{}' was not created correctly!", sprite.textureName);
				return;
			}

			glm::vec4 spriteRect{ transform.position.x, transform.position.y, sprite.width, sprite.height };
			glm::vec4 uvRect{ sprite.uvs.u, sprite.uvs.v, sprite.uvs.uv_width, sprite.uvs.uv_height };

			glm::mat4 model = TRSModel(transform, sprite.width, sprite.height);

			if (sprite.isIsometric)
			{
				m_BatchRenderer->AddSpriteIso(spriteRect,
											  uvRect,
											  pTexture->GetID(),
											  sprite.isoCellX,
											  sprite.isoCellY,
											  sprite.layer,
											  model,
											  sprite.color);
			}
			else
			{
				m_BatchRenderer->AddSprite(spriteRect, uvRect, pTexture->GetID(), sprite.layer, model, sprite.color);
			}
		}

		m_BatchRenderer->End();
		m_BatchRenderer->Render();

		spriteShader->Disable();
	}

}
