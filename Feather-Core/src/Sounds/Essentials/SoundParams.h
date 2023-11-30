#pragma once

#include <string>

namespace Feather {

	struct SoundParams
	{
		std::string name{ "" }, description{ "" }, filename{ "" };
		double duration{ 0.0 };
	};

}
