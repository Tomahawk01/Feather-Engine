#pragma once
#include <string>

namespace Feather {

	struct SaveProject;

	class ProjectLoader
	{
	public:
		bool CreateNewProject(const std::string& projectName, const std::string& filepath);
		bool LoadProject(const std::string& filepath);
		bool SaveLoadedProject(SaveProject& save);

	private:
		bool CreateProjectFile(const std::string& projectName, const std::string& filepath);
		bool CreateMainLuaScript(const std::string& projectName, const std::string& filepath);
	};

}
