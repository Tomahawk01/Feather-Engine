#pragma once

#include <spdlog/spdlog.h>

#include <memory>

namespace Feather {
	
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
	};

}

// Strip logging from DISTribution builds
#ifdef DIST
#define F_TRACE(...)
#define F_INFO(...)
#define F_WARN(...)
#define F_ERROR(...)
#define F_FATAL(...)
#else
#define F_TRACE(...) ::Feather::Log::GetLogger()->trace(__VA_ARGS__)
#define F_INFO(...)	 ::Feather::Log::GetLogger()->info(__VA_ARGS__)
#define F_WARN(...)  ::Feather::Log::GetLogger()->warn(__VA_ARGS__)
#define F_ERROR(...) ::Feather::Log::GetLogger()->error(__VA_ARGS__)
#define F_FATAL(...) ::Feather::Log::GetLogger()->critical(__VA_ARGS__)
#endif
