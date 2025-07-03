#pragma once

#include <string>

namespace Feather {

	struct SoundParams
	{
		std::string name{};
		std::string description{};
		std::string filename{};
		double duration{ 0.0 };
	};

}
