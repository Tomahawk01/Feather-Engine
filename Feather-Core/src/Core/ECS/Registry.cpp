#include "Registry.h"

namespace Feather {

	Registry::Registry()
		: m_Registry{ nullptr }
	{
		m_Registry = std::make_unique<entt::registry>();
	}

}
