#include "AnimationSystem.h"

#include "Logger/Logger.h"
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

			if (!registry.GetRegistry().all_of<UIComponent>(entity) && !EntityInView(transform, sprite.width, sprite.height, camera))
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

	void AnimationSystem::CreateAnimationSystemLuaBind(sol::state& lua, Registry& registry)
	{
		auto& camera = registry.GetContext<std::shared_ptr<Camera2D>>();

		F_ASSERT(camera && "A camera must exist in the current scene!");

		lua.new_usertype<AnimationSystem>(
			"AnimationSystem",
			sol::call_constructor,
			sol::constructors<AnimationSystem()>(),
			"update",
			[&](AnimationSystem& system, Registry& reg) { system.Update(reg, *camera); }
		);
	}

}
