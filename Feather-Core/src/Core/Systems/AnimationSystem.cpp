#include "AnimationSystem.h"

#include <SDL.h>

#include "Core/CoreUtils/CoreUtilities.h"

namespace Feather {

	AnimationSystem::AnimationSystem(Registry& registry)
		: m_Registry{registry}
	{}

	void AnimationSystem::Update()
	{
		auto view = m_Registry.GetRegistry().view<AnimationComponent, SpriteComponent, TransformComponent>();
		if (view.size_hint() < 1)
			return;

		auto& camera = m_Registry.GetContext<std::shared_ptr<Camera2D>>();

		for (auto entity : view)
		{
			const auto& transform = view.get<TransformComponent>(entity);
			auto& sprite = view.get<SpriteComponent>(entity);
			auto& animation = view.get<AnimationComponent>(entity);

			if (!EntityInView(transform, sprite.width, sprite.height, *camera))
				continue;

			if (animation.numFrames <= 0)
				continue;
			if (!animation.isLooped && animation.currentFrame >= animation.numFrames - 1)
				continue;

			animation.currentFrame = ((SDL_GetTicks() - animation.startTime) * animation.frameRate / 1000) % animation.numFrames;

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
