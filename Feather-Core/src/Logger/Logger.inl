#pragma once
#include "Logger/Logger.h"

#include <chrono>
#include <ctime>
#include <format>
#include <iostream>
#include <sstream>

#include <Windows.h>
constexpr WORD GREEN = 2;
constexpr WORD RED = 4;
constexpr WORD YELLOW = 6;
constexpr WORD WHITE = 7;

namespace Feather {

	template <typename... Args>
	void Log::Trace(const std::string_view message, Args&&... args)
	{
		assert(m_Initialized && "Logger must be initialized before it is used!");

		if (!m_Initialized)
		{
			std::cout << "Logger must be initialized before it is used!" << std::endl;
			return;
		}

		std::stringstream ss;
		ss << CurrentDateTime() << " Feather [TRACE]: " << std::vformat(message, std::make_format_args(args...));

		if (m_ConsoleLog)
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, WHITE);
			std::cout << ss.str() << "\n";
			SetConsoleTextAttribute(hConsole, WHITE);
		}

		if (m_RetainLogs)
		{
			m_LogEntries.emplace_back(LogEntry::LogType::TRACE, ss.str());
			m_LogAdded = true;
		}
	}

	template <typename... Args>
	void Log::Info(const std::string_view message, Args&&... args)
	{
		assert(m_Initialized && "Logger must be initialized before it is used!");

		if (!m_Initialized)
		{
			std::cout << "Logger must be initialized before it is used!" << std::endl;
			return;
		}

		std::stringstream ss;
		ss << CurrentDateTime() << " Feather [INFO]: " << std::vformat(message, std::make_format_args(args...));

		if (m_ConsoleLog)
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, GREEN);
			std::cout << ss.str() << "\n";
			SetConsoleTextAttribute(hConsole, WHITE);
		}

		if (m_RetainLogs)
		{
			m_LogEntries.emplace_back(LogEntry::LogType::INFO, ss.str());
			m_LogAdded = true;
		}
	}

	template <typename... Args>
	void Log::Warn(const std::string_view message, Args&&... args)
	{
		assert(m_Initialized && "Logger must be initialized before it is used!");

		if (!m_Initialized)
		{
			std::cout << "Logger must be initialized before it is used!" << std::endl;
			return;
		}

		std::stringstream ss;
		ss << CurrentDateTime() << " Feather [WARN]: " << std::vformat(message, std::make_format_args(args...));

		if (m_ConsoleLog)
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, YELLOW);
			std::cout << ss.str() << "\n";
			SetConsoleTextAttribute(hConsole, WHITE);
		}

		if (m_RetainLogs)
		{
			m_LogEntries.emplace_back(LogEntry::LogType::WARN, ss.str());
			m_LogAdded = true;
		}
	}

	template <typename... Args>
	void Log::Error(const std::string_view message, Args&&... args)
	{
		assert(m_Initialized && "Logger must be initialized before it is used!");

		if (!m_Initialized)
		{
			std::cout << "Logger must be initialized before it is used!" << std::endl;
			return;
		}

		std::stringstream ss;
		ss << CurrentDateTime() << " Feather [ERROR]: " << std::vformat(message, std::make_format_args(args...));

		if (m_ConsoleLog)
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, RED);
			std::cout << ss.str() << "\n";
			SetConsoleTextAttribute(hConsole, WHITE);
		}

		if (m_RetainLogs)
		{
			m_LogEntries.emplace_back(LogEntry::LogType::ERR, ss.str());
			m_LogAdded = true;
		}
	}

	template <typename... Args>
	void Log::Critical(std::source_location location, const std::string_view message, Args&&... args)
	{
		assert(m_Initialized && "Logger must be initialized before it is used!");

		if (!m_Initialized)
		{
			std::cout << "Logger must be initialized before it is used!" << std::endl;
			return;
		}

		std::stringstream ss;
		ss << CurrentDateTime() << " Feather [CRIT]: " << std::vformat(message, std::make_format_args(args...)) << "\nFUNC: " << location.function_name() << "\nLINE: " << location.line();

		if (m_ConsoleLog)
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, RED);
			std::cout << ss.str() << "\n";
			SetConsoleTextAttribute(hConsole, WHITE);
		}

		if (m_RetainLogs)
		{
			m_LogEntries.emplace_back(LogEntry::LogType::CRITICAL, ss.str());
			m_LogAdded = true;
		}
	}

}
