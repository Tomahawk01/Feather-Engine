#pragma once

#include <spdlog/spdlog.h>

#include <memory>

namespace Feather {
	
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }
		inline static std::shared_ptr<spdlog::logger>& GetLuaLogger() { return s_LuaLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
		static std::shared_ptr<spdlog::logger> s_LuaLogger;
	};

}

// Strip logging from DISTribution builds
#ifdef DIST
#define F_TRACE(...)
#define F_INFO(...)
#define F_WARN(...)
#define F_ERROR(...)
#define F_FATAL(...)

#define F_LUA_TRACE(...)
#define F_LUA_INFO(...)
#define F_LUA_WARN(...)
#define F_LUA_ERROR(...)
#else
#define F_TRACE(...)		::Feather::Log::GetLogger()->trace(__VA_ARGS__)
#define F_INFO(...)			::Feather::Log::GetLogger()->info(__VA_ARGS__)
#define F_WARN(...)			::Feather::Log::GetLogger()->warn(__VA_ARGS__)
#define F_ERROR(...)		::Feather::Log::GetLogger()->error(__VA_ARGS__)
#define F_FATAL(...)		::Feather::Log::GetLogger()->critical(__VA_ARGS__)

#define F_LUA_TRACE(...)	::Feather::Log::GetLuaLogger()->trace(__VA_ARGS__)
#define F_LUA_INFO(...)		::Feather::Log::GetLuaLogger()->info(__VA_ARGS__)
#define F_LUA_WARN(...)		::Feather::Log::GetLuaLogger()->warn(__VA_ARGS__)
#define F_LUA_ERROR(...)	::Feather::Log::GetLuaLogger()->error(__VA_ARGS__)
#endif

#define F_ASSERT(x) assert(x)
