#pragma once
#include <string>
#include <string_view>
#include <source_location>
#include <vector>
#include <cassert>

namespace Feather {

	struct LogEntry
	{
		enum class LogType
		{
			TRACE,
			INFO,
			WARN,
			ERR,
			CRITICAL,
			NONE
		};
		LogType type{ LogType::TRACE };
		std::string log{ "" };
	};

	class Log
	{
	public:
		static Log& GetInstance();

		~Log() = default;
		// Make the logger non-copyable
		Log(const Log&) = delete;
		Log& operator=(const Log&) = delete;

		void Init(bool consoleLog = true, bool retainLogs = true);

		template <typename... Args>
		void Trace(const std::string_view message, Args&&... args);
		template <typename... Args>
		void Info(const std::string_view message, Args&&... args);
		template <typename... Args>
		void Warn(const std::string_view message, Args&&... args);
		template <typename... Args>
		void Error(const std::string_view message, Args&&... args);
		template <typename... Args>
		void Critical(std::source_location location, const std::string_view message, Args&&... args);

		void LuaTrace(const std::string_view message);
		void LuaInfo(const std::string_view message);
		void LuaWarn(const std::string_view message);
		void LuaError(const std::string_view message);

		inline void ClearLogs() { m_LogEntries.clear(); }
		inline const std::vector<LogEntry>& GetLogs() { return m_LogEntries; }
		inline void ResetLogAdded() { m_LogAdded = false; }
		inline const bool LogAdded() const { return m_LogAdded; }

	private:
		std::vector<LogEntry> m_LogEntries;
		bool m_LogAdded{ false }, m_Initialized{ false }, m_ConsoleLog{ true }, m_RetainLogs{ true };

		Log() = default;

		struct LogTime
		{
			std::string time;
			LogTime(const std::string& date);
		};

		std::string CurrentDateTime();
	};

}

#ifdef DIST
#define F_TRACE(...)
#define F_INFO(...)
#define F_WARN(...)
#define F_ERROR(...)
#define F_FATAL(...)
#else
#define F_TRACE(...)		Feather::Log::GetInstance().Trace(__VA_ARGS__)
#define F_INFO(...)			Feather::Log::GetInstance().Info(__VA_ARGS__)
#define F_WARN(...)			Feather::Log::GetInstance().Warn(__VA_ARGS__)
#define F_ERROR(...)		Feather::Log::GetInstance().Error(__VA_ARGS__)
#define F_FATAL(...)		Feather::Log::GetInstance().Critical(std::source_location::current(), __VA_ARGS__)
#endif

#define F_ASSERT(x) assert(x)

#define F_INIT_LOGS(console, retain)	Feather::Log::GetInstance().Init(console, retain)
#define F_LOG_ADDED()					Feather::Log::GetInstance().LogAdded()
#define F_RESET_ADDED()					Feather::Log::GetInstance().ResetLogAdded()
#define F_GET_LOGS()					Feather::Log::GetInstance().GetLogs()
#define F_CLEAR_LOGS()					Feather::Log::GetInstance().ClearLogs()

#include "Logger/Logger.inl"
