#pragma once

#include <entt.hpp>

#include <string>

namespace Feather {

	class Registry;

	entt::entity FindEntityByTag(Registry& registry, const std::string& tag);

}
