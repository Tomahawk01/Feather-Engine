#include "Logger/Logger.h"

namespace Feather {

	Log::LogTime::LogTime(const std::string& date)
		: time{ date.substr(11, 8) }
	{}

	std::string Log::CurrentDateTime()
	{
		auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		char buf[30];
		ctime_s(buf, sizeof(buf), &time);
		LogTime logTime{ std::string{ buf } };
		return std::format("[{0}]", logTime.time);
	}

	Log& Log::GetInstance()
	{
		static Log instance{};
		return instance;
	}

	void Log::Init(bool consoleLog, bool retainLogs)
	{
		assert(!m_Initialized && "Don not call Initialize more than once!");

		if (m_Initialized)
		{
			std::cout << "Logger has already been initialized!" << std::endl;
			return;
		}

		m_ConsoleLog = consoleLog;
		m_RetainLogs = retainLogs;
		m_Initialized = true;
	}

	void Log::LuaTrace(const std::string_view message)
	{
		std::scoped_lock lock{ m_Mutex };
		assert(m_Initialized && "Logger must be initialized before it is used!");

		if (!m_Initialized)
		{
			std::cout << "Logger must be initialized before it is used!" << std::endl;
			return;
		}

		std::stringstream ss;
		ss << CurrentDateTime() << " Lua [TRACE]: " << message << "\n";

		if (m_ConsoleLog)
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, WHITE);
			std::cout << ss.str();
			SetConsoleTextAttribute(hConsole, WHITE);
		}

		if (m_RetainLogs)
		{
			m_LogEntries.emplace_back(LogEntry::LogType::TRACE, ss.str());
		}
	}

	void Log::LuaInfo(const std::string_view message)
	{
		std::scoped_lock lock{ m_Mutex };
		assert(m_Initialized && "Logger must be initialized before it is used!");

		if (!m_Initialized)
		{
			std::cout << "Logger must be initialized before it is used!" << std::endl;
			return;
		}

		std::stringstream ss;
		ss << CurrentDateTime() << " Lua [INFO]: " << message << "\n";

		if (m_ConsoleLog)
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, GREEN);
			std::cout << ss.str();
			SetConsoleTextAttribute(hConsole, WHITE);
		}

		if (m_RetainLogs)
		{
			m_LogEntries.emplace_back(LogEntry::LogType::INFO, ss.str());
		}
	}

	void Log::LuaWarn(const std::string_view message)
	{
		std::scoped_lock lock{ m_Mutex };
		assert(m_Initialized && "Logger must be initialized before it is used!");

		if (!m_Initialized)
		{
			std::cout << "Logger must be initialized before it is used!" << std::endl;
			return;
		}

		std::stringstream ss;
		ss << CurrentDateTime() << " Lua [WARN]: " << message << "\n";

		if (m_ConsoleLog)
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, YELLOW);
			std::cout << ss.str();
			SetConsoleTextAttribute(hConsole, WHITE);
		}

		if (m_RetainLogs)
		{
			m_LogEntries.emplace_back(LogEntry::LogType::WARN, ss.str());
		}
	}

	void Log::LuaError(const std::string_view message)
	{
		std::scoped_lock lock{ m_Mutex };
		assert(m_Initialized && "Logger must be initialized before it is used!");

		if (!m_Initialized)
		{
			std::cout << "Logger must be initialized before it is used!" << std::endl;
			return;
		}

		std::stringstream ss;
		ss << CurrentDateTime() << " Lua [ERROR]: " << message << "\n";

		if (m_ConsoleLog)
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, RED);
			std::cout << ss.str();
			SetConsoleTextAttribute(hConsole, WHITE);
		}

		if (m_RetainLogs)
		{
			m_LogEntries.emplace_back(LogEntry::LogType::ERR, ss.str());
		}
	}

}
