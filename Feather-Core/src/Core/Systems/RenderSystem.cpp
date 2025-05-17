#include "RenderSystem.h"

#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Core/BatchRenderer.h"
#include "Renderer/Essentials/Shader.h"
#include "Utils/HelperUtilities.h"

#include <ranges>

namespace Feather {

	RenderSystem::RenderSystem()
		: m_BatchRenderer{ std::make_unique<SpriteBatchRenderer>() }
	{}

	void RenderSystem::Update(Registry& registry, Camera2D& camera, const std::vector<SpriteLayerParams>& layerFilters)
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

		// Enable shader
		spriteShader->Enable();
		spriteShader->SetUniformMat4("uProjection", cam_mat);

		m_BatchRenderer->Begin();

		auto spriteView = registry.GetRegistry().view<SpriteComponent, TransformComponent>(entt::exclude<UIComponent>);
		std::function<bool(entt::entity)> filterFunc;

		// Check if layers are visible, if not filter them out
		if (layerFilters.empty())
		{
			filterFunc = [](entt::entity) { return true; };
		}
		else
		{
			filterFunc = [&](entt::entity entity) {
				// Only filter tiles
				if (!registry.GetRegistry().all_of<TileComponent>(entity))
					return true;

				const auto& sprite = spriteView.get<SpriteComponent>(entity);
				if (sprite.layer >= 0 && sprite.layer < layerFilters.size())
					return layerFilters[sprite.layer].isVisible;

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
				F_ERROR("Texture '{0}' was not created correctly!", sprite.textureName);
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

	void RenderSystem::CreateRenderSystemLuaBind(sol::state& lua, Registry& registry)
	{
		auto& camera = registry.GetContext<std::shared_ptr<Camera2D>>();

		F_ASSERT(camera && "A camera must exist in the current scene!");

		lua.new_usertype<RenderSystem>(
			"RenderSystem",
			sol::call_constructor,
			sol::constructors<RenderSystem()>(),
			"update",
			[&](RenderSystem& system, Registry& reg) { system.Update(reg, *camera); }
		);
	}

}
