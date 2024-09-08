#include "FileDialog.h"

#include <tinyfiledialogs.h>

namespace Feather {

	std::string FileDialog::OpenFileDialog(const std::string& title, const std::string& defaultPath, const std::vector<const char*>& filters, const std::string& filterDesc)
	{
		const char* file = tinyfd_openFileDialog(title.c_str(), defaultPath.c_str(), filters.size(), filters.data(), filterDesc.c_str(), 1);
		if (!file)
			return std::string{};

		return std::string{ file };
	}

	std::string FileDialog::SaveFileDialog(const std::string& title, const std::string& defaultPath, const std::vector<const char*>& filters, const std::string& filterDesc)
	{
		const char* file = tinyfd_saveFileDialog(title.c_str(), defaultPath.c_str(), filters.size(), filters.data(), filterDesc.c_str());
		if (!file)
			return std::string{};

		return std::string{ file };
	}

	std::string FileDialog::SelectFolderDialog(const std::string& title, const std::string& defaultPath)
	{
		const char* folder = tinyfd_selectFolderDialog(title.c_str(), defaultPath.c_str());
		if (!folder)
			return std::string{};

		return std::string{ folder };
	}

}
