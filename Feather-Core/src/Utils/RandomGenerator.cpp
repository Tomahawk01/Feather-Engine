#include "RandomGenerator.h"

namespace Feather {

    RandomGenerator::RandomGenerator()
        : RandomGenerator(0, std::numeric_limits<uint32_t>::max())
    {}

    RandomGenerator::RandomGenerator(uint32_t min, uint32_t max)
        : m_MTEngine{}, m_Distribution{ min, max }
    {
        Initialize();
    }

    float RandomGenerator::GetFloat()
    {
        return (float)m_Distribution(m_MTEngine);
    }

    uint32_t RandomGenerator::GetInt()
    {
        return m_Distribution(m_MTEngine);
    }

    void RandomGenerator::Initialize()
    {
        m_MTEngine.seed(std::random_device()());
    }

}
