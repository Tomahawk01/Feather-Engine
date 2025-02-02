#pragma once

#include <string>

namespace Feather {

	struct SaveProject
	{
		/* The name of the project */
		std::string projectName{ "" };
		/* The active path of the project */
		std::string projectPath{ "" };
		/* The active project file path */
		std::string projectFilePath{ "" };
		/* The main lua script path of the project */
		std::string mainLuaScript{ "" };
	};

}
