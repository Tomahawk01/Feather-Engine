#include "ToolManager.h"
#include "CreateTileTool.h"
#include "RectFillTool.h"
#include "ToolAccessories.h"

namespace Feather {

	ToolManager::ToolManager()
	{
		m_mapTools.emplace(ToolType::CREATE_TILE, std::make_unique<CreateTileTool>());
		m_mapTools.emplace(ToolType::RECT_FILL_TILE, std::make_unique<RectFillTool>());

		// TODO: Add other tools as needed

		SetToolActive(ToolType::RECT_FILL_TILE);
	}

	void ToolManager::Update(Canvas& canvas)
	{
		auto activeTool = std::ranges::find_if(m_mapTools, [](const auto& tool) { return tool.second->IsActivated(); });
		if (activeTool != m_mapTools.end())
			activeTool->second->Update(canvas);
	}

	void ToolManager::SetToolActive(ToolType toolType)
	{
		for (const auto& [type, tool] : m_mapTools)
		{
			if (type == toolType)
				tool->Activate();
			else
				tool->Deactivate();
		}
	}

	TileTool* ToolManager::GetActiveTool()
	{
		auto activeTool = std::ranges::find_if(m_mapTools, [](const auto& tool) { return tool.second->IsActivated(); });
		if (activeTool != m_mapTools.end())
			return activeTool->second.get();

		return nullptr;
	}

	void ToolManager::EnableGridSnap(bool enable)
	{
		for (auto& tool : m_mapTools)
		{
			if (enable)
				tool.second->EnableGridSnap();
			else
				tool.second->DisableGridSnap();
		}
	}

}
