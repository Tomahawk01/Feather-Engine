#include "AbstractTool.h"
#include "Logger/Logger.h"
#include "Core/Scripting/InputManager.h"
#include "Core/ECS/Registry.h"
#include "Renderer/Core/Camera2D.h"
#include "../Utilities/EditorUtilities.h"

namespace Feather {

	AbstractTool::AbstractTool()
		: m_MouseScreenCoords{ 0.0f }, m_MouseWorldCoords{ 0.0f },
		m_GUICursorCoords{ 0.0f }, m_GUIRelativeCoords{ 0.0f },
		m_WindowPos{ 0.0f }, m_WindowSize{ 0.0f },
		m_Activated{ false }, m_OutOfBounds{ false }, m_OverTileMapWindow{ false }
	{}

	void AbstractTool::Update(Canvas& canvas)
	{
		UpdateMouseWorldCoords();
		CheckOutOfBounds(canvas);
	}

	bool AbstractTool::SetupTool(Registry* registry, Camera2D* camera)
	{
		if (!registry)
		{
			F_ERROR("Failed to setup tool: Registry was nullptr");
			return false;
		}
		if (!camera)
		{
			F_ERROR("Failed to setup tool: Camera was nullptr");
			return false;
		}

		m_Camera = camera;
		m_Registry = registry;

		return true;
	}

	void AbstractTool::UpdateMouseWorldCoords()
	{
		m_MouseScreenCoords = m_GUICursorCoords - m_GUIRelativeCoords;
		if (!m_Camera)
			return;

		m_MouseWorldCoords = m_Camera->ScreenCoordsToWorld(m_MouseScreenCoords);
	}

	void AbstractTool::CheckOutOfBounds(const Canvas& canvas)
	{
		auto boundsWidth{ canvas.width - canvas.tileWidth * 0.5f };
		auto boundsHeight{ canvas.height - canvas.tileHeight * 0.5f };

		if (m_MouseWorldCoords.x <= boundsWidth && m_MouseWorldCoords.y <= boundsHeight &&
			m_MouseWorldCoords.x >= 0.0f && m_MouseWorldCoords.y >= 0.0f)
		{
			m_OutOfBounds = false;
		}
		else
		{
			m_OutOfBounds = true;
		}
	}

	bool AbstractTool::MouseButtonJustPressed(MouseButton button)
	{
		return INPUT_MANAGER().GetMouse().IsButtonJustPressed(static_cast<int>(button));
	}

	bool AbstractTool::MouseButtonJustReleased(MouseButton button)
	{
		return INPUT_MANAGER().GetMouse().IsButtonJustReleased(static_cast<int>(button));
	}

	bool AbstractTool::MouseButtonPressed(MouseButton button)
	{
		return INPUT_MANAGER().GetMouse().IsButtonPressed(static_cast<int>(button));
	}

	bool AbstractTool::MouseMoving()
	{
		return INPUT_MANAGER().GetMouse().IsMouseMoving();
	}
}
