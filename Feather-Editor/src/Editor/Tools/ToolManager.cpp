#include "ToolManager.h"
#include "CreateTileTool.h"
#include "RectFillTool.h"
#include "ToolAccessories.h"

#include "Renderer/Core/Camera2D.h"

#include "Editor/Scene/SceneObject.h"
#include "Editor/Tools/Gizmos/TranslateGizmo.h"
#include "Editor/Tools/Gizmos/ScaleGizmo.h"
#include "Editor/Tools/Gizmos/RotateGizmo.h"

namespace Feather {

	ToolManager::ToolManager()
	{
		m_mapTools.emplace(ToolType::CREATE_TILE, std::make_unique<CreateTileTool>());
		m_mapTools.emplace(ToolType::RECT_FILL_TILE, std::make_unique<RectFillTool>());

		m_mapGizmos.emplace(GizmoType::TRANSLATE, std::make_unique<TranslateGizmo>());
		m_mapGizmos.emplace(GizmoType::SCALE, std::make_unique<ScaleGizmo>());
		m_mapGizmos.emplace(GizmoType::ROTATE, std::make_unique<RotateGizmo>());

		// TODO: Add other tools as needed

		SetToolActive(ToolType::RECT_FILL_TILE);
	}

	void ToolManager::Update(Canvas& canvas)
	{
		auto activeTool = std::ranges::find_if(m_mapTools, [](const auto& tool) { return tool.second->IsActivated(); });
		if (activeTool != m_mapTools.end())
			activeTool->second->Update(canvas);

		auto activeGizmo = std::ranges::find_if(m_mapGizmos, [](const auto& gizmo) { return gizmo.second->IsActivated(); });
		if (activeGizmo != m_mapGizmos.end())
			activeGizmo->second->Update(canvas);
	}

	void ToolManager::SetToolActive(ToolType toolType)
	{
		// Deactivate all gizmos when map exists
		for (const auto& [type, gizmo] : m_mapGizmos)
		{
			gizmo->Deactivate();
			gizmo->Hide();
		}

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
		for (const auto& [type, gizmo] : m_mapGizmos)
		{
			if (type == gizmoType)
			{
				gizmo->Activate();
				gizmo->Show();
			}
			else
			{
				gizmo->Deactivate();
				gizmo->Hide();
			}
		}

		m_ActiveGizmoType = gizmoType;
	}

	void ToolManager::SetSelectedEntity(entt::entity entity)
	{
		for (auto& [type, gizmo] : m_mapGizmos)
		{
			gizmo->SetSelectedEntity(entity);
		}
	}

	TileTool* ToolManager::GetActiveTool()
	{
		auto activeTool = std::ranges::find_if(m_mapTools, [](const auto& tool) { return tool.second->IsActivated(); });
		if (activeTool != m_mapTools.end())
			return activeTool->second.get();

		return nullptr;
	}

	Gizmo* ToolManager::GetActiveGizmo()
	{
		auto activeGizmo = std::ranges::find_if(m_mapGizmos, [](const auto& gizmo) { return gizmo.second->IsActivated(); });
		if (activeGizmo != m_mapGizmos.end())
			return activeGizmo->second.get();

		return nullptr;
	}

	AbstractTool* ToolManager::GetActiveToolFromAbstract()
	{
		if (auto* tool = GetActiveTool())
			return tool;

		if (auto* gizmo = GetActiveGizmo())
			return gizmo;

		return nullptr;
	}

	bool ToolManager::SetupTools(SceneObject* sceneObject, Camera2D* camera)
	{
		for (auto& [type, tool] : m_mapTools)
		{
			if (!tool->SetupTool(sceneObject, camera))
				return false;
		}

		for (auto& [type, gizmo] : m_mapGizmos)
		{
			if (!gizmo->SetupTool(sceneObject, camera))
				return false;
		}

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
