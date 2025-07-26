#pragma once

#include "AbstractCommandManager.h"
#include "cmds/CreateTileToolCmds.h"
#include "cmds/RectToolCmds.h"
#include "cmds/TileCmds.h"
// TODO: Add other commands as needed

namespace Feather {

	using UndoableCommands = VarCommands<CreateTileToolAddCmd, CreateTileToolRemoveCmd,
										 RectToolAddTilesCmd, RectToolRemoveTilesCmd,
										 RemoveTileLayerCmd, AddTileLayerCmd, ChangeTileLayerNameCmd, MoveTileLayerCmd/* Add other commad types */>;

	class CommandManager : public AbstractCommandManager<CreateTileToolAddCmd, CreateTileToolRemoveCmd,
														 RectToolAddTilesCmd, RectToolRemoveTilesCmd,
														 RemoveTileLayerCmd, AddTileLayerCmd, ChangeTileLayerNameCmd, MoveTileLayerCmd /* Add other commad types */>
	{
	};

}
