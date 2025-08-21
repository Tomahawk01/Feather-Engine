#pragma once

#include <entt.hpp>

namespace Feather {

	class Registry;

	entt::entity FindEntityByTag(Registry& registry, const std::string& tag);

}
