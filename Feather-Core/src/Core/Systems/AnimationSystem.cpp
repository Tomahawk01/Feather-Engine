#include "AnimationSystem.h"

#include <SDL.h>

#include "Core/ECS/Components/AnimationComponent.h"
#include "Core/ECS/Components/SpriteComponent.h"
#include "Core/ECS/Components/TransformComponent.h"

namespace Feather {

	AnimationSystem::AnimationSystem(Registry& registry)
		: m_Registry{registry}
	{}

	void AnimationSystem::Update()
	{
		auto view = m_Registry.GetRegistry().view<AnimationComponent, SpriteComponent, TransformComponent>();

		for (auto entity : view)
		{
			const auto& transform = view.get<TransformComponent>(entity);
			auto& sprite = view.get<SpriteComponent>(entity);
			auto& animation = view.get<AnimationComponent>(entity);

			animation.currentFrame = (SDL_GetTicks() * animation.frameRate / 1000) % animation.numFrames;

			if (animation.isVertical)
			{
				sprite.uvs.v = animation.currentFrame * sprite.uvs.uv_width;
				sprite.uvs.u = animation.frameOffset * sprite.uvs.uv_width;
			}
			else
			{
				sprite.uvs.u = (animation.currentFrame * sprite.uvs.uv_width) + (animation.frameOffset * sprite.uvs.uv_width);
			}
		}
	}

}
