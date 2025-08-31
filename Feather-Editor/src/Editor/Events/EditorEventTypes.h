#pragma once

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
		FileAction action{ FileAction::NoAction };
		std::string filepath{};
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
		/* Create new folder */
		Folder,
		/* Create new lua file with F_Class */
		LuaClass,
		/* Create new lua file with an empty lua table. TableName = {} */
		LuaTable,
		/* Create new lua file with F_Class that has a Feather State class implementation */
		LuaStateClass,
		/* Create an empty lua file */
		EmptyLuaFile,

		NoAction
	};

	struct ContentCreateEvent
	{
		ContentCreateAction action{ ContentCreateAction::NoAction };
		std::string filepath{};
	};

	struct NameChangeEvent
	{
		std::string oldName{};
		std::string newName{};
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
