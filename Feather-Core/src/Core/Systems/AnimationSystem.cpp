#include "AnimationSystem.h"
#include "Core/ECS/Registry.h"
#include "Core/CoreUtils/CoreUtilities.h"

#include <SDL.h>

namespace Feather {

	void AnimationSystem::Update(Registry& registry, Camera2D& camera)
	{
		auto view = registry.GetRegistry().view<AnimationComponent, SpriteComponent, TransformComponent>();
		if (view.size_hint() < 1)
			return;

		for (auto entity : view)
		{
			const auto& transform = view.get<TransformComponent>(entity);
			auto& sprite = view.get<SpriteComponent>(entity);
			auto& animation = view.get<AnimationComponent>(entity);

			if (!EntityInView(transform, sprite.width, sprite.height, camera))
				continue;

			if (animation.numFrames <= 0)
				continue;
			if (!animation.isLooped && animation.currentFrame >= animation.numFrames - 1)
				continue;

			animation.currentFrame = ((SDL_GetTicks() - animation.startTime) * animation.frameRate / 1000) % animation.numFrames;

			if (animation.isVertical)
			{
				sprite.uvs.v = animation.currentFrame * sprite.uvs.uv_height;
				sprite.uvs.u = animation.frameOffset * sprite.uvs.uv_width;
			}
			else
			{
				sprite.uvs.u = (animation.currentFrame * sprite.uvs.uv_width) + (animation.frameOffset * sprite.uvs.uv_width);
			}
		}
	}

}
