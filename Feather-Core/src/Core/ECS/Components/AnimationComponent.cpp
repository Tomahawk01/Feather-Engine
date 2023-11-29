#include "AnimationComponent.h"

#include <entt.hpp>

void Feather::AnimationComponent::CreateAnimationLuaBind(sol::state& lua)
{
	lua.new_usertype<AnimationComponent>(
		"Animation",
		"type_id", &entt::type_hash<AnimationComponent>::value,
		sol::call_constructor,
		sol::factories([](int numFrames, int frameRate, int frameOffset, bool isVertical, bool isLooped)
		{
			return AnimationComponent {
					.numFrames = numFrames,
					.frameRate = frameRate,
					.frameOffset = frameOffset,
					.isVertical = isVertical,
					.isLooped = isLooped,
			};
		}),
		"num_frames", &AnimationComponent::numFrames,
		"frame_rate", &AnimationComponent::frameRate,
		"frame_offset", &AnimationComponent::frameOffset,
		"current_frame", &AnimationComponent::currentFrame,
		"start_time", &AnimationComponent::startTime,
		"is_vertical", &AnimationComponent::isVertical,
		"is_looped", &AnimationComponent::isLooped,
		"reset", [](AnimationComponent& anim)
		{
			anim.currentFrame = 0;
			anim.startTime = SDL_GetTicks();
		}
	);
}
