#include "CreateTileToolCmds.h"

#include "Logger/Logger.h"
#include "Core/ECS/Registry.h"
#include "Core/ECS/Entity.h"
#include "Core/ECS/Components/AllComponents.h"

#include "Editor/Utilities/EditorUtilities.h"

namespace Feather {

	void CreateTileToolAddCmd::undo()
	{
		F_TRACE("Undo Create Tile Tool Command!");
	}

	void CreateTileToolAddCmd::redo()
	{
		F_TRACE("Redo Create Tile Tool Command!");
	}

}
