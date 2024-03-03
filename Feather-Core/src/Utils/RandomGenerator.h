#pragma once

#include <random>

namespace Feather {

	class RandomGenerator
	{
	public:
		RandomGenerator();
		RandomGenerator(uint32_t min, uint32_t max);

		float GetFloat();
		uint32_t GetInt();

	private:
		std::mt19937 m_MTEngine;
		std::uniform_int_distribution<std::mt19937::result_type> m_Distribution;

		void Initialize();
	};

}
