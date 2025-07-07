#include "ScriptCompiler.h"

#include "Core/CoreUtils/CoreEngineData.h"
#include "Utils/HelperUtilities.h"
#include "FileSystem/Utilities/FileSystemUtilities.h"
#include "Logger/Logger.h"

#include <sol/sol.hpp>

#include <filesystem>
#include <algorithm>
#include <array>

#ifdef _WIN32
constexpr const char* FIND_LUAC_COMMAND = "where luac";
#else
constexpr const char* FIND_LUAC_COMMAND = "which luac";
#endif

namespace fs = std::filesystem;

namespace Feather {

	ScriptCompiler::ScriptCompiler()
		: m_OutFile{}
		, m_LuaFiles{}
		, m_LuacPath{ std::nullopt }
	{
		m_LuacPath = FindLuaCompiler();
		if (!m_LuacPath)
		{
			throw std::runtime_error("Script compiler error: Lua Compiler (luac) not found on system!\n"
									 "Please install the luac and add to your environment.");
		}
	}

	ScriptCompiler::~ScriptCompiler() = default;

	bool ScriptCompiler::AddScripts(const std::string& scriptList)
	{
		if (!fs::exists(fs::path{ scriptList }))
		{
			F_ERROR("Failed to add scripts. Script list file {} is invalid", scriptList);
			return false;
		}

		sol::state lua{};

		try
		{
			auto result = lua.safe_script_file(scriptList);
			if (!result.valid())
			{
				sol::error error = result;
				throw error;
			}

			sol::optional<sol::table> optScriptList = lua["ScriptList"];
			if (!optScriptList)
			{
				F_ERROR("Failed to add scripts. {} is missing \"ScriptList\" table or is invalid", scriptList);
				return false;
			}

			const std::string contentPath = std::format("{}content{}", CORE_GLOBALS().GetProjectPath(), PATH_SEPARATOR);

			for (const auto& [_, script] : *optScriptList)
			{
				std::string filepath{ std::format("{}{}", contentPath, script.as<std::string>()) };
				if (!fs::exists(fs::path{ filepath }))
				{
					F_ERROR("Failed to add script: {} does not exist", filepath);
					return false;
				}

				m_LuaFiles.push_back(filepath);
			}
		}
		catch (const sol::error& error)
		{
			F_ERROR("Failed to add scripts: ", error.what());
			return false;
		}

		return true;
	}

	bool ScriptCompiler::AddScript(const std::string& script)
	{
		if (!fs::exists(fs::path{ script }))
		{
			F_ERROR("Failed to add script: {} does not exist", script);
			return false;
		}

		m_LuaFiles.push_back(script);

		return true;
	}

	void ScriptCompiler::Compile()
	{
		if (m_LuaFiles.empty())
		{
			throw std::invalid_argument("Script compiler error: No Lua files provided for compilation!");
		}

		auto notExist = std::ranges::find_if(
			m_LuaFiles,
			[](const std::string& file)
			{
				return !fs::exists(fs::path{ file });
			}
		);

		if (notExist != m_LuaFiles.end())
		{
			throw std::runtime_error("Script compiler error: File not found - " + *notExist);
		}

#ifdef _WIN32
		std::string command = std::format("cmd /C \"\"{}\" \"-o\" \"{}\"", *m_LuacPath, m_OutFile);
#else
		std::string command = std::format("\"\"{}\" \"-o\" \"{}\"", *m_LuacPath, m_OutFile);
#endif

		for (const auto& file : m_LuaFiles)
		{
			command += " " + std::string{ "\"" + file + "\"" };
		}

		command += "\"";

		const std::string error = ExecCmdWithErrorOutput(command);
		if (!error.empty())
		{
			throw std::runtime_error(std::format("Script compiler error: Lua compilation failed. Error: {}", error));
		}

		F_INFO("Successfully compiled lua files in {}", m_OutFile);
	}

	std::optional<std::string> ScriptCompiler::FindLuaCompiler()
	{
#ifdef _WIN32
		std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(std::string{ FIND_LUAC_COMMAND }.c_str(), "r"), _pclose);
#else
		std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(std::string{ cmd + " 2>&1" }.c_str(), "r"), pclose);
#endif

		if (!pipe)
			return std::nullopt;

		std::string result{};
		std::array<char, 256> buffer;

		if (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
		{
			result = buffer.data();
			result.erase(result.find_last_not_of(" \n\r") + 1);
		}

		return result.empty() ? std::nullopt : std::optional<std::string>{ result };
	}

}
