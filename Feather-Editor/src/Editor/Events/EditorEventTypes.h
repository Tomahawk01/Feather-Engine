#pragma once
#include <string>

namespace Feather {

	class Entity;

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

	struct CloseEditorEvent
	{
		// ...
	};

	struct SwitchEntityEvent
	{
		Entity* entity{ nullptr };
	};

	enum class EContentCreateAction
	{
		Folder,
		LuaClass,
		LuaTable,

		NoAction
	};

	struct ContentCreateEvent
	{
		EContentCreateAction eAction{ EContentCreateAction::NoAction };
		std::string sFilepath{ "" };
	};

}
