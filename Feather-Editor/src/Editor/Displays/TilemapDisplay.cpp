#include "TilemapDisplay.h"

#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Core/Systems/RenderSystem.h"
#include "Core/Systems/RenderUISystem.h"
#include "Core/Systems/RenderShapeSystem.h"
#include "Core/Systems/AnimationSystem.h"
#include "Core/Scripting/InputManager.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Core/Renderer.h"
#include "Windowing/Input/Mouse.h"
#include "Logger/Logger.h"

#include "Editor/Systems/GridSystem.h"
#include "Editor/Utilities/EditorFramebuffers.h"
#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Utilities/ImGuiUtils.h"
#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"
#include "Editor/Scene/SceneManager.h"
#include "Editor/Scene/SceneObject.h"
#include "Editor/Tools/ToolManager.h"
#include "Editor/Tools/ToolAccessories.h"
#include "Editor/Tools/CreateTileTool.h"
#include "Editor/Commands/CommandManager.h"

#include "imgui.h"

namespace Feather {

	TilemapDisplay::TilemapDisplay()
		: m_TilemapCam{ std::make_unique<Camera2D>() }
	{}

	void TilemapDisplay::Draw()
	{
		if (!ImGui::Begin("Tilemap Editor"))
		{
			ImGui::End();
			return;
		}

		DrawToolbar();
		RenderTilemap();

		auto& mainRegistry = MAIN_REGISTRY();

		if (ImGui::BeginChild("##tilemap", ImVec2{0, 0}, false, ImGuiWindowFlags_NoScrollWithMouse))
		{
			auto& editorFramebuffers = mainRegistry.GetContext<std::shared_ptr<EditorFramebuffers>>();
			const auto& fb = editorFramebuffers->mapFramebuffers[FramebufferType::TILEMAP];

			ImVec2 imageSize{ static_cast<float>(fb->GetWidth()), static_cast<float>(fb->GetHeight()) };
			ImVec2 windowSize{ ImGui::GetWindowSize() };

			float x = (windowSize.x - imageSize.x) * 0.5f;
			float y = (windowSize.y - imageSize.y) * 0.5f;

			ImGui::SetCursorPos(ImVec2{ x,y });
			ImGuiIO io = ImGui::GetIO();
			auto relativePos = ImGui::GetCursorScreenPos();
			auto windowPos = ImGui::GetWindowPos();

			auto activeTool = SCENE_MANAGER().GetToolManager().GetActiveTool();
			if (activeTool)
			{
				activeTool->SetRelativeCoords(glm::vec2{ relativePos.x, relativePos.y });
				activeTool->SetCursorCoords(glm::vec2{ io.MousePos.x, io.MousePos.y });
				activeTool->SetWindowPos(glm::vec2{ windowPos.x, windowPos.y });
				activeTool->SetWindowSize(glm::vec2{ windowSize.x, windowSize.y });

				activeTool->SetOverTilemapWindow(ImGui::IsWindowHovered());
			}

			ImGui::Image((ImTextureID)(intptr_t)fb->GetTextureID(), imageSize, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });

			// Accept scene drop target
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DROP_SCENE_SRC);
				if (payload)
				{
					SCENE_MANAGER().SetCurrentScene(std::string{ (const char*)payload->Data });
					LoadNewScene();
					m_TilemapCam->Reset();
				}

				ImGui::EndDragDropTarget();
			}

			// Check for resize based on window size
			if (fb->GetWidth() != static_cast<int>(windowSize.x) || fb->GetHeight() != static_cast<int>(windowSize.y))
			{
				fb->Resize(static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
				m_TilemapCam->Resize(static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
			}

			ImGui::EndChild();
		}

		ImGui::End();
	}

	void TilemapDisplay::Update()
	{
		auto currentScene = SCENE_MANAGER().GetCurrentScene();
		if (!currentScene)
			return;

		auto activeTool = SCENE_MANAGER().GetToolManager().GetActiveTool();
		if (activeTool && activeTool->IsOverTilemapWindow() && !ImGui::GetDragDropPayload())
		{
			PanZoomCamera(activeTool->GetMouseScreenCoords());
			activeTool->Update(currentScene->GetCanvas());
			activeTool->Create();
		}

		auto& mainRegistry = MAIN_REGISTRY();
		auto& animationSystem = mainRegistry.GetAnimationSystem();
		animationSystem.Update(currentScene->GetRegistry(), *m_TilemapCam);

		m_TilemapCam->Update();

		auto& keyboard = INPUT_MANAGER().GetKeyboard();
		if (keyboard.IsKeyPressed(F_KEY_LCTRL) &&
			keyboard.IsKeyJustPressed(F_KEY_Z))
		{
			if (keyboard.IsKeyPressed(F_KEY_LSHIFT))
				COMMAND_MANAGER().Redo();
			else
				COMMAND_MANAGER().Undo();
		}
	}

	void TilemapDisplay::RenderTilemap()
	{
		auto currentScene = SCENE_MANAGER().GetCurrentScene();
		auto& mainRegistry = MAIN_REGISTRY();
		auto& editorFramebuffers = mainRegistry.GetContext<std::shared_ptr<EditorFramebuffers>>();
		auto& renderer = mainRegistry.GetContext<std::shared_ptr<Renderer>>();

		auto& renderSystem = mainRegistry.GetRenderSystem();
		auto& renderUISystem = mainRegistry.GetRenderUISystem();
		auto& renderShapeSystem = mainRegistry.GetRenderShapeSystem();

		const auto& fb = editorFramebuffers->mapFramebuffers[FramebufferType::TILEMAP];

		fb->Bind();
		renderer->SetViewport(0, 0, fb->GetWidth(), fb->GetHeight());
		renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		renderer->ClearBuffers(true, true, false);

		if (!currentScene)
		{
			fb->Unbind();
			return;
		}

		auto& gridSystem = mainRegistry.GetContext<std::shared_ptr<GridSystem>>();
		gridSystem->Update(*currentScene, *m_TilemapCam);

		renderSystem.Update(currentScene->GetRegistry(), *m_TilemapCam, currentScene->GetLayerParams());
		if (CORE_GLOBALS().RenderCollidersEnabled())
			renderShapeSystem.Update(currentScene->GetRegistry(), *m_TilemapCam);
		renderUISystem.Update(currentScene->GetRegistry());

		auto activeTool = SCENE_MANAGER().GetToolManager().GetActiveTool();
		if (activeTool)
			activeTool->Draw();
		fb->Unbind();
		fb->CheckResize();

		renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		renderer->ClearBuffers(true, true, false);
	}

	void TilemapDisplay::LoadNewScene()
	{
		auto currentScene = SCENE_MANAGER().GetCurrentScene();
		if (!currentScene)
			return;

		auto& toolManager = TOOL_MANAGER();

		if (!toolManager.SetupTools(currentScene.get(), m_TilemapCam.get()))
		{
			F_ASSERT(false && "This should work!?");
			__debugbreak();
		}

		if (!SCENE_MANAGER().GetCurrentTileset().empty())
			toolManager.SetToolsCurrentTileset(SCENE_MANAGER().GetCurrentTileset());
	}

	void TilemapDisplay::PanZoomCamera(const glm::vec2& mousePos)
	{
		if (!m_TilemapCam)
			return;

		auto& mouse = INPUT_MANAGER().GetMouse();
		if (!mouse.IsButtonJustPressed(F_MOUSE_MIDDLE) && !mouse.IsButtonPressed(F_MOUSE_MIDDLE) && mouse.GetMouseWheelY() == 0)
			return;

		static glm::vec2 startPosition{ 0.0f };
		auto screenOffset = m_TilemapCam->GetScreenOffset();
		bool isOffsetChanged{ false }, isScaleChanged{ false };

		if (mouse.IsButtonJustPressed(F_MOUSE_MIDDLE))
			startPosition = mousePos;

		if (mouse.IsButtonPressed(F_MOUSE_MIDDLE))
		{
			screenOffset += (mousePos - startPosition);
			isOffsetChanged = true;
		}

		glm::vec2 currentWorldPos = m_TilemapCam->ScreenCoordsToWorld(mousePos);
		float scale = m_TilemapCam->GetScale();

		if (mouse.GetMouseWheelY() == 1)
		{
			scale += 0.2f;
			isScaleChanged = true;
			isOffsetChanged = true;
		}
		else if (mouse.GetMouseWheelY() == -1)
		{
			scale -= 0.2f;
			isScaleChanged = true;
			isOffsetChanged = true;
		}

		scale = std::clamp(scale, 1.0f, 10.0f);

		if (isScaleChanged)
			m_TilemapCam->SetScale(scale);

		glm::vec2 afterMovePos = m_TilemapCam->ScreenCoordsToWorld(mousePos);

		screenOffset += (afterMovePos - currentWorldPos);
		if (isOffsetChanged)
			m_TilemapCam->SetScreenOffset(screenOffset);

		startPosition = mousePos;
	}

	void TilemapDisplay::DrawToolbar()
	{
		ImGui::Separator();

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });

		auto& commandManager = COMMAND_MANAGER();
		if (commandManager.UndoEmpty())
		{
			ImGui::DisabledButton(ICON_FA_UNDO, TOOL_BUTTON_SIZE, "Undo [CTRL + Z] - Nothing to undo");
		}
		else
		{
			if (ImGui::Button(ICON_FA_UNDO, TOOL_BUTTON_SIZE))
				commandManager.Undo();
		}
		ImGui::ItemToolTip("Undo [CTRL + Z]");
		ImGui::SameLine();

		if (commandManager.RedoEmpty())
		{
			ImGui::DisabledButton(ICON_FA_REDO, TOOL_BUTTON_SIZE, "Redo [CTRL + SHIFT + Z] - Nothing to redo");
		}
		else
		{
			if (ImGui::Button(ICON_FA_REDO, TOOL_BUTTON_SIZE))
				commandManager.Redo();
		}
		ImGui::ItemToolTip("Redo [CTRL + SHIFT + Z]");
		ImGui::SameLine(0.0f, 32.0f);

		auto& toolManager = TOOL_MANAGER();
		const ToolType activeToolType = toolManager.GetActiveToolType();
		const GizmoType activeGizmoType = toolManager.GetActiveGizmoType();

		ImGui::DisabledButton(ICON_FA_TOOLS, TOOL_BUTTON_SIZE);

		ImGui::SameLine();

		if (activeGizmoType == GizmoType::TRANSLATE)
		{
			ImGui::ActiveButton(ICON_FA_ARROWS_ALT, TOOL_BUTTON_SIZE);
		}
		else
		{
			if (ImGui::Button(ICON_FA_ARROWS_ALT, TOOL_BUTTON_SIZE))
				toolManager.SetGizmoActive(GizmoType::TRANSLATE);
		}
		ImGui::ItemToolTip("Translate [W] - move game objects");

		ImGui::SameLine();

		if (activeGizmoType == GizmoType::SCALE)
		{
			ImGui::ActiveButton(ICON_FA_EXPAND, TOOL_BUTTON_SIZE);
		}
		else
		{
			if (ImGui::Button(ICON_FA_EXPAND, TOOL_BUTTON_SIZE))
				toolManager.SetGizmoActive(GizmoType::SCALE);
		}
		ImGui::ItemToolTip("Scale [E] - scale game objects");

		ImGui::SameLine();

		if (activeGizmoType == GizmoType::ROTATE)
		{
			//ImGui::ActiveButton(ICON_FA_CIRCLE_NOTCH, TOOL_BUTTON_SIZE);
			ImGui::DisabledButton(ICON_FA_CIRCLE_NOTCH, TOOL_BUTTON_SIZE, "Rotate [R] - rotate game objects. Currently Unavailable");
		}
		else
		{
			/*if (ImGui::Button(ICON_FA_CIRCLE_NOTCH, TOOL_BUTTON_SIZE))
				toolManager.SetGizmoActive(GizmoType::ROTATE);*/
			ImGui::DisabledButton(ICON_FA_CIRCLE_NOTCH, TOOL_BUTTON_SIZE, "Rotate [R] - rotate game objects. Currently Unavailable");
		}
		//ImGui::ItemToolTip("Rotate [R] - rotate game objects. Currently Unavailable");

		ImGui::SameLine();

		if (activeToolType == ToolType::CREATE_TILE)
		{
			ImGui::ActiveButton(ICON_FA_STAMP, TOOL_BUTTON_SIZE);
		}
		else
		{
			if (ImGui::Button(ICON_FA_STAMP, TOOL_BUTTON_SIZE))
				toolManager.SetToolActive(ToolType::CREATE_TILE);
		}
		ImGui::ItemToolTip("Create Tile [T] - creates a single tile");

		ImGui::SameLine();

		if (activeToolType == ToolType::RECT_FILL_TILE)
		{
			ImGui::ActiveButton(ICON_FA_CHESS_BOARD, TOOL_BUTTON_SIZE);
		}
		else
		{
			if (ImGui::Button(ICON_FA_CHESS_BOARD, TOOL_BUTTON_SIZE))
				toolManager.SetToolActive(ToolType::RECT_FILL_TILE);
		}
		ImGui::ItemToolTip("Rect Fill tool [Y] - creates tiles inside of rectangle");

		ImGui::SameLine();

		ImGui::DisabledButton(ICON_FA_TOOLS, TOOL_BUTTON_SIZE);

		ImGui::PopStyleVar(2);
		ImGui::Separator();
		ImGui::AddSpaces(1);
	}

}
