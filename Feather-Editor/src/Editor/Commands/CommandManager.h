#pragma once

#include "AbstractCommandManager.h"
#include "cmds/CreateTileToolCmds.h"
// TODO: Add other commands as needed

namespace Feather {

	using UndoableCommands = VarCommands<CreateTileToolAddCmd /* Add other commad types */>;

	class CommandManager : public AbstractCommandManager<CreateTileToolAddCmd /* Add other commad types */>
	{

	};

}
