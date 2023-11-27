#include "AnimationComponent.h"

#include <entt.hpp>

void Feather::AnimationComponent::CreateAnimationLuaBind(sol::state& lua)
{
	lua.new_usertype<AnimationComponent>(
		"Animation",
		"type_id", &entt::type_hash<AnimationComponent>::value,
		sol::call_constructor,
		sol::factories([](int numFrames, int frameRate, int frameOffset, bool isVertical)
		{
			return AnimationComponent {
					.numFrames = numFrames,
					.frameRate = frameRate,
					.frameOffset = frameOffset,
					.isVertical = isVertical
			};
		}),
		"num_frames", &AnimationComponent::numFrames,
		"frame_rate", &AnimationComponent::frameRate,
		"num_frames", &AnimationComponent::frameOffset,
		"current_frame", &AnimationComponent::currentFrame,
		"is_vertical", &AnimationComponent::isVertical
	);
}
