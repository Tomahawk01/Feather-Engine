#pragma once

namespace Feather {

	class ProjectInfo;

	class ProjectLoader
	{
	public:
		bool CreateNewProject(const std::string& projectName, const std::string& filepath);
		bool LoadProject(const std::string& filepath);
		bool SaveLoadedProject(const ProjectInfo& projectInfo);

	private:
		bool CreateProjectFile(const std::string& projectName, const std::string& filepath);
		bool CreateMainLuaScript(const std::string& projectName, const std::string& filepath);
		bool CreateScriptListFile();
	};

}
