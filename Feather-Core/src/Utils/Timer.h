#pragma once

using namespace std::chrono;

namespace Feather {

	class Timer
	{
	public:
		Timer() = default;
		~Timer() = default;

		void Start();
		void Stop();
		void Pause();
		void Resume();

		const int64_t ElapsedMS();
		const int64_t ElapsedSec();

		inline const bool IsRunning() const { return m_IsRunning; }
		inline const bool IsPaused() const { return m_IsPaused; }

	private:
		time_point<steady_clock> m_StartPoint;
		time_point<steady_clock> m_PausedPoint;
		bool m_IsRunning{ false };
		bool m_IsPaused{ false };
	};

}
