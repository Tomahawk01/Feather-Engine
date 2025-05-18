#include "AnimationComponent.h"

#include <entt.hpp>

std::string Feather::AnimationComponent::to_string() const
{
	std::stringstream ss;
	ss << "==== Animation Component ==== \n" << std::boolalpha <<
		  "Num Frames: " << numFrames << "\n" <<
		  "Frame Rate: " << frameRate << "\n" <<
		  "Frame Offset: " << frameOffset << "\n" <<
		  "IsVertical: " << isVertical << "\n" <<
		  "IsLooped: " << isLooped << "\n";

	return ss.str();
}

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
		"numFrames", &AnimationComponent::numFrames,
		"frameRate", &AnimationComponent::frameRate,
		"frameOffset", &AnimationComponent::frameOffset,
		"currentFrame", &AnimationComponent::currentFrame,
		"startTime", &AnimationComponent::startTime,
		"isVertical", &AnimationComponent::isVertical,
		"isLooped", &AnimationComponent::isLooped,
		"reset", [](AnimationComponent& anim)
		{
			anim.currentFrame = 0;
			anim.startTime = SDL_GetTicks();
		},
		"toString", &AnimationComponent::to_string
	);
}
