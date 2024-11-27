#pragma once

#include <variant>
#include <stack>

namespace Feather {

	template <typename T>
	concept CommandType = requires(T t)
	{
		{
			t.undo()
		} -> std::same_as<void>;
		{
			t.redo()
		} -> std::same_as<void>;
	};

	template <CommandType... Cmds>
	using VarCommands = std::variant<Cmds...>;

	template <CommandType... Cmds>
	using CommandStack = std::stack<VarCommands<Cmds...>>;

	template <CommandType... Cmds>
	class AbstractCommandManager
	{
	public:
		void Undo();
		void Redo();
		void Clear();

		void Execute(VarCommands<Cmds...>& undoableVariant);

	private:
		void RunUndo(VarCommands<Cmds...>& undoVariant);
		void RunRedo(VarCommands<Cmds...>& redoVariant);

		template <CommandType TCmd>
		void RunUndo_Impl(TCmd& undoCmd);

		template <CommandType TCmd>
		void RunRedo_Impl(TCmd& redoCmd);

	private:
		CommandStack<Cmds...> m_UndoStack;
		CommandStack<Cmds...> m_RedoStack;
	};

}

#include "AbstractCommandManager.inl"
