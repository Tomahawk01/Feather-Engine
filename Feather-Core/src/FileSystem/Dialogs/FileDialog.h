#pragma once
#include <string>
#include <vector>

namespace Feather {

	class FileDialog
	{
	public:
		std::string OpenFileDialog(const std::string& title = "Open", const std::string& defaultPath = "",
								   const std::vector<const char*>& filters = {}, const std::string& filterDesc = "");
		std::string SaveFileDialog(const std::string& title = "Save", const std::string& defaultPath = "",
								   const std::vector<const char*>& filters = {}, const std::string& filterDesc = "");
		std::string SelectFolderDialog(const std::string& title = "Select Folder", const std::string& defaultPath = "");
	};

}
