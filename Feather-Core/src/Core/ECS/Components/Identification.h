#pragma once

#include <entt.hpp>

#include <string>

namespace Feather {

	struct Identification
	{
		std::string name{ "GameObject" };
		std::string group{};
		uint32_t entity_id{ entt::null };
	};

}
