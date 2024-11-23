#include "RandomGenerator.h"

namespace Feather {

    // ============= Int =============

    RandomIntGenerator::RandomIntGenerator()
        : RandomIntGenerator(0, std::numeric_limits<uint32_t>::max())
    {}

    RandomIntGenerator::RandomIntGenerator(uint32_t min, uint32_t max)
        : m_MTEngine{}, m_IntDistribution{ min, max }
    {
        Initialize();
    }

    int RandomIntGenerator::GetValue()
    {
        return static_cast<int>(m_IntDistribution(m_MTEngine));
    }

    void RandomIntGenerator::Initialize()
    {
        m_MTEngine.seed(std::random_device()());
    }

    // ============= Float =============

    RandomFloatGenerator::RandomFloatGenerator()
        : RandomFloatGenerator(0.0f, std::numeric_limits<float>::max())
    {}

    RandomFloatGenerator::RandomFloatGenerator(float min, float max)
        : m_MTEngine{}, m_FloatDistribution{ min, max }
    {
        Initialize();
    }

    float RandomFloatGenerator::GetValue()
    {
        return m_FloatDistribution(m_MTEngine);
    }

    void RandomFloatGenerator::Initialize()
    {
        m_MTEngine.seed(std::random_device()());
    }

}
