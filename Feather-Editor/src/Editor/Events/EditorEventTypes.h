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

	enum class ContentCreateAction
	{
		Folder,
		LuaClass,
		LuaTable,
		EmptyLuaFile,

		NoAction
	};

	struct ContentCreateEvent
	{
		ContentCreateAction eAction{ ContentCreateAction::NoAction };
		std::string sFilepath{ "" };
	};

	struct NameChangeEvent
	{
		std::string oldName{ "" };
		std::string newName{ "" };
		Entity* entity{ nullptr };
	};

	enum class ComponentType
	{
		Transform,
		Sprite,
		Physics,
		Text,
		BoxCollider,
		CircleCollider,
		RigidBody,
		Animation,
		Tile,
		UI,

		NoType
	};

	struct AddComponentEvent
	{
		Entity* entity{ nullptr };
		ComponentType type{ ComponentType::NoType };
	};

	ComponentType GetComponentTypeFromStr(const std::string& componentStr);
	std::string GetComponentStrFromType(ComponentType type);

}
