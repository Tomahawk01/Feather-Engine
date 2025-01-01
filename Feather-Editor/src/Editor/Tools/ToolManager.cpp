#include "ToolManager.h"
#include "CreateTileTool.h"
#include "RectFillTool.h"
#include "ToolAccessories.h"

#include "Renderer/Core/Camera2D.h"
#include "Editor/Scene/SceneObject.h"

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
		// TODO: Deactivate all gizmos when map exists
		m_ActiveGizmoType = GizmoType::NO_GIZMO;

		for (const auto& [type, tool] : m_mapTools)
		{
			if (type == toolType)
				tool->Activate();
			else
				tool->Deactivate();
		}

		m_ActiveToolType = toolType;
	}

	void ToolManager::SetToolsCurrentTileset(const std::string& tileset)
	{
		for (auto& [eType, pTool] : m_mapTools)
		{
			pTool->LoadSpriteTextureData(tileset);
		}
	}

	void ToolManager::SetTileToolStartCoords(int x, int y)
	{
		for (auto& [eType, pTool] : m_mapTools)
		{
			pTool->SetSpriteUVs(x, y);
		}
	}

	void ToolManager::SetGizmoActive(GizmoType gizmoType)
	{
		// Deactivate all tools
		for (const auto& [type, tool] : m_mapTools)
		{
			tool->Deactivate();
		}
		m_ActiveToolType = ToolType::NO_TOOL;

		// Activate the specified Gizmo
		// TODO: Create gizmo map and set active

		m_ActiveGizmoType = gizmoType;
	}

	TileTool* ToolManager::GetActiveTool()
	{
		auto activeTool = std::ranges::find_if(m_mapTools, [](const auto& tool) { return tool.second->IsActivated(); });
		if (activeTool != m_mapTools.end())
			return activeTool->second.get();

		return nullptr;
	}

	bool ToolManager::SetupTools(SceneObject* sceneObject, Camera2D* camera)
	{
		for (auto& [type, tool] : m_mapTools)
		{
			if (!tool->SetupTool(sceneObject, camera))
				return false;
		}

		// TODO: Setup Gizmos

		return true;
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
