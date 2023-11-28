#include "Timer.h"

namespace Feather {

    void Timer::Start()
    {
        if (!m_IsRunning)
        {
            m_StartPoint = steady_clock::now();
            m_IsRunning = true;
            m_IsPaused = false;
        }
    }

    void Timer::Stop()
    {
        if (m_IsRunning)
            m_IsRunning = false;
    }

    void Timer::Pause()
    {
        if (m_IsRunning && !m_IsPaused)
        {
            m_IsPaused = true;
            m_PausedPoint = steady_clock::now();
        }
    }

    void Timer::Resume()
    {
        if (m_IsRunning && m_IsPaused)
        {
            m_IsPaused = false;
            m_StartPoint += duration_cast<milliseconds>(steady_clock::now() - m_PausedPoint);
        }
    }

    const int64_t Timer::ElapsedMS()
    {
        if (m_IsRunning)
        {
            if (m_IsPaused)
                return duration_cast<milliseconds>(m_PausedPoint - m_StartPoint).count();
            else
                return duration_cast<milliseconds>(steady_clock::now() - m_StartPoint).count();
        }

        return 0;
    }

    const int64_t Timer::ElapsedSec()
    {
        return ElapsedMS() / 1000;
    }

}
