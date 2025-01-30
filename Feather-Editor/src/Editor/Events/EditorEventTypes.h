#pragma once
#include <string>

namespace Feather {

	enum class FileAction
	{
		Delete,
		Copy,
		Cut,
		Paste,
		Rename,
		FileDropped,
		NoAction
	};

	struct FileEvent
	{
		FileAction eAction{ FileAction::NoAction };
		std::string sFilepath{ "" };
	};

}
