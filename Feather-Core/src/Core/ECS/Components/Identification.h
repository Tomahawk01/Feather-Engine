#pragma once

#include <string>

namespace Feather {

	struct Identification
	{
		std::string name{ "GameObject" }, group{ "" };
		int32_t entity_id{ -1 };
	};

}
