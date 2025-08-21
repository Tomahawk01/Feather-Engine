#pragma once

namespace Feather {

	class ScriptCompiler
	{
	public:
		ScriptCompiler();
		~ScriptCompiler();

		bool AddScripts(const std::string& scriptList);
		bool AddScript(const std::string& script);

		void Compile();

		inline void SetOutputFileName(const std::string& outFile) { m_OutFile = outFile; }
		inline void ClearScripts() { m_LuaFiles.clear(); }

	private:
		std::optional<std::string> FindLuaCompiler();

	private:
		std::string m_OutFile;
		std::vector<std::string> m_LuaFiles;
		std::optional<std::string> m_LuacPath;
	};

}
