#include "TilemapDisplay.h"

#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/Resources/AssetManager.h"
#include "Core/Systems/RenderSystem.h"
#include "Core/Systems/RenderUISystem.h"
#include "Core/Systems/RenderShapeSystem.h"
#include "Core/Systems/RenderPickingSystem.h"
#include "Core/Systems/AnimationSystem.h"
#include "Core/Scripting/InputManager.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/CoreUtils/Prefab.h"
#include "Core/Events/EventDispatcher.h"
#include "Core/Events/EngineEventTypes.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Core/Renderer.h"
#include "Renderer/Essentials/PickingTexture.h"
#include "Windowing/Input/Mouse.h"
#include "Logger/Logger.h"

#include "Editor/Systems/GridSystem.h"
#include "Editor/Utilities/EditorFramebuffers.h"
#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"
#include "Editor/Scene/SceneManager.h"
#include "Editor/Scene/SceneObject.h"
#include "Editor/Tools/ToolManager.h"
#include "Editor/Tools/ToolAccessories.h"
#include "Editor/Tools/CreateTileTool.h"
#include "Editor/Tools/Gizmos/Gizmo.h"
#include "Editor/Commands/CommandManager.h"
#include "Editor/Events/EditorEventTypes.h"

#include "imgui.h"

namespace Feather {

	TilemapDisplay::TilemapDisplay()
		: m_TilemapCam{ std::make_unique<Camera2D>() }
		, m_WindowActive{ false }
	{
		ADD_EVENT_HANDLER(KeyEvent, &TilemapDisplay::HandleKeyPressedEvent, *this);
	}

	void TilemapDisplay::Draw()
	{
		if (!ImGui::Begin(ICON_FA_MAP " Tilemap Editor"))
		{
			ImGui::End();
			return;
		}

		DrawToolbar();
		RenderTilemap();

		auto& mainRegistry = MAIN_REGISTRY();

		if (ImGui::BeginChild("##tilemap", ImVec2{0, 0}, false, ImGuiWindowFlags_NoScrollWithMouse))
		{
			m_WindowActive = ImGui::IsWindowFocused();

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

			auto activeTool = TOOL_MANAGER().GetActiveToolFromAbstract();
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
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::string{ DROP_SCENE_SRC }.c_str()))
				{
					SCENE_MANAGER().UnloadCurrentScene();
					SCENE_MANAGER().SetCurrentScene(std::string{ (const char*)payload->Data });
					SCENE_MANAGER().LoadCurrentScene();
					LoadNewScene();
					m_TilemapCam->Reset();
				}
				else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::string{ DROP_PREFAB_SRC }.c_str()))
				{
					if (auto prefab = ASSET_MANAGER().GetPrefab(std::string{ (const char*)payload->Data }))
					{
						const auto& prefabbed = prefab->GetPrefabbedEntity();
						AddPrefabbedEntityToScene(prefabbed);
					}
				}

				ImGui::EndDragDropTarget();
			}

			// Check for resize based on window size
			if (fb->GetWidth() != static_cast<int>(windowSize.x) || fb->GetHeight() != static_cast<int>(windowSize.y))
			{
				fb->Resize(static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));

				auto& pickingTexture = mainRegistry.GetContext<std::shared_ptr<PickingTexture>>();
				if (pickingTexture)
					pickingTexture->Resize(static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));

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

		auto activeTool = TOOL_MANAGER().GetActiveTool();
		if (activeTool && activeTool->IsOverTilemapWindow() && !ImGui::GetDragDropPayload())
		{
			PanZoomCamera(activeTool->GetMouseScreenCoords());
			activeTool->Update(currentScene->GetCanvas());
			activeTool->Create();
		}

		auto activeGizmo = TOOL_MANAGER().GetActiveGizmo();
		if (activeGizmo && activeGizmo->IsOverTilemapWindow() && !ImGui::GetDragDropPayload())
		{
			PanZoomCamera(activeGizmo->GetMouseScreenCoords());
			activeGizmo->Update(currentScene->GetCanvas());
		}

		if (CORE_GLOBALS().AnimationRenderEnabled())
		{
			auto& mainRegistry = MAIN_REGISTRY();
			auto& animationSystem = mainRegistry.GetAnimationSystem();
			animationSystem.Update(currentScene->GetRegistry(), *m_TilemapCam);
		}

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

		auto activeGizmo = TOOL_MANAGER().GetActiveGizmo();
		auto& mouse = INPUT_MANAGER().GetMouse();

		if (activeGizmo && activeGizmo->IsOverTilemapWindow() && !activeGizmo->OverGizmo() &&
			!ImGui::GetDragDropPayload() && mouse.IsButtonJustPressed(F_MOUSE_LEFT))
		{
			auto& renderPickingSystem = mainRegistry.GetContext<std::shared_ptr<RenderPickingSystem>>();
			// Handle the picking texture/system
			if (renderPickingSystem && currentScene)
			{
				auto& pickingTexture = mainRegistry.GetContext<std::shared_ptr<PickingTexture>>();
				if (pickingTexture)
				{
					renderer->SetCapability(Renderer::GLCapability::BLEND, false);
					pickingTexture->Bind();
					renderer->SetViewport(0, 0, pickingTexture->GetWidth(), pickingTexture->GetHeight());
					renderer->SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
					renderer->ClearBuffers(true, true);

					renderPickingSystem->Update(currentScene->GetRegistry(), *m_TilemapCam);

					const auto& pos = activeGizmo->GetMouseScreenCoords();
					auto id = static_cast<entt::entity>(pickingTexture->ReadPixel(static_cast<int>(pos.x), static_cast<int>(pos.y)));

					if (!currentScene->GetRegistry().IsValid(static_cast<entt::entity>(id)))
					{
						id = entt::null;
					}
					else
					{
						Entity checkedEntity{ currentScene->GetRegistry(), static_cast<entt::entity>(id) };
						if (checkedEntity.HasComponent<TileComponent>())
							id = entt::null;
					}

					SCENE_MANAGER().GetToolManager().SetSelectedEntity(id);
				}

				pickingTexture->Unbind();
				pickingTexture->CheckResize();
				renderer->SetCapability(Renderer::GLCapability::BLEND, true);
			}
		}

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

		auto activeTool = TOOL_MANAGER().GetActiveTool();
		if (activeTool)
			activeTool->Draw();

		if (activeGizmo)
			activeGizmo->Draw(renderUISystem.GetCamera());

		fb->Unbind();
		fb->CheckResize();

		renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		renderer->ClearBuffers(true, true, false);
	}

	void TilemapDisplay::LoadNewScene()
	{
		auto currentScene = SCENE_MANAGER().GetCurrentSceneObject();
		if (!currentScene)
			return;

		auto& toolManager = TOOL_MANAGER();

		if (!toolManager.SetupTools(currentScene, m_TilemapCam.get()))
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

		if (isScaleChanged)
		{
			scale = std::clamp(scale, 1.0f, 10.0f);
			m_TilemapCam->SetScale(scale);
		}

		if (isOffsetChanged)
		{
			glm::vec2 afterMovePos = m_TilemapCam->ScreenCoordsToWorld(mousePos);
			screenOffset += (afterMovePos - currentWorldPos);
			m_TilemapCam->SetScreenOffset(screenOffset);
		}

		startPosition = mousePos;
	}

	void TilemapDisplay::HandleKeyPressedEvent(const KeyEvent& keyEvent)
	{
		if (!m_WindowActive || keyEvent.type == EKeyEventType::Released)
			return;

		// No need to change the tools if there is no scene loaded
		auto currentScene = SCENE_MANAGER().GetCurrentScene();
		if (!currentScene)
			return;

		if (keyEvent.key == F_KEY_W)
		{
			TOOL_MANAGER().SetGizmoActive(GizmoType::TRANSLATE);
		}
		else if (keyEvent.key == F_KEY_E)
		{
			TOOL_MANAGER().SetGizmoActive(GizmoType::SCALE);
		}
		else if (keyEvent.key == F_KEY_R)
		{
			TOOL_MANAGER().SetGizmoActive(GizmoType::ROTATE);
		}
		else if (keyEvent.key == F_KEY_T)
		{
			TOOL_MANAGER().SetToolActive(ToolType::CREATE_TILE);
		}
		else if (keyEvent.key == F_KEY_Y)
		{
			// IsoGrid scenes are not currently supported for rect tool
			if (currentScene->GetMapType() == EMapType::Grid)
			{
				TOOL_MANAGER().SetToolActive(ToolType::RECT_FILL_TILE);
			}
		}
	}

	void TilemapDisplay::AddPrefabbedEntityToScene(const PrefabbedEntity& prefabbed)
	{
		auto pCurrentScene = SCENE_MANAGER().GetCurrentSceneObject();
		if (!pCurrentScene)
			return;

		int count{ 1 };
		std::string sTag{ prefabbed.id->name };
		while (pCurrentScene->CheckTagName(sTag))
		{
			sTag = prefabbed.id->name + std::to_string(count);
			++count;
		}

		Entity newEnt{ pCurrentScene->GetRegistry(), sTag, prefabbed.id->group };

		newEnt.AddComponent<TransformComponent>(prefabbed.transform);
		if (prefabbed.sprite)
		{
			newEnt.AddComponent<SpriteComponent>(prefabbed.sprite.value());
		}

		if (prefabbed.animation)
		{
			newEnt.AddComponent<AnimationComponent>(prefabbed.animation.value());
		}

		if (prefabbed.boxCollider)
		{
			newEnt.AddComponent<BoxColliderComponent>(prefabbed.boxCollider.value());
		}

		if (prefabbed.circleCollider)
		{
			newEnt.AddComponent<CircleColliderComponent>(prefabbed.circleCollider.value());
		}

		if (prefabbed.textComp)
		{
			newEnt.AddComponent<TextComponent>(prefabbed.textComp.value());
		}

		if (prefabbed.physics)
		{
			newEnt.AddComponent<PhysicsComponent>(prefabbed.physics.value());
		}

		pCurrentScene->AddGameObjectByTag(sTag, newEnt.GetEntity());
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

		ImGui::DisabledButton(ICON_FA_TOOLS "##1", TOOL_BUTTON_SIZE);

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
			ImGui::ActiveButton(ICON_FA_CIRCLE_NOTCH, TOOL_BUTTON_SIZE);
		}
		else
		{
			if (ImGui::Button(ICON_FA_CIRCLE_NOTCH, TOOL_BUTTON_SIZE))
				toolManager.SetGizmoActive(GizmoType::ROTATE);
		}
		ImGui::ItemToolTip("Rotate [R] - rotates game objects");

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

		bool isIsoScene{ false };
		if (auto pCurrentScene = SCENE_MANAGER().GetCurrentScene())
		{
			isIsoScene = pCurrentScene->GetMapType() == EMapType::IsoGrid;
		}

		if (isIsoScene)
		{
			ImGui::DisabledButton(ICON_FA_CHESS_BOARD, TOOL_BUTTON_SIZE, "Rect Tile Tool [Y] - Isometric grids not currently supported.");
		}
		else
		{
			if (activeToolType == ToolType::RECT_FILL_TILE)
			{
				ImGui::ActiveButton(ICON_FA_CHESS_BOARD, TOOL_BUTTON_SIZE);
			}
			else
			{
				if (ImGui::Button(ICON_FA_CHESS_BOARD, TOOL_BUTTON_SIZE))
				{
					toolManager.SetToolActive(ToolType::RECT_FILL_TILE);
				}
			}

			ImGui::ItemToolTip("Rect Tile Tool [Y] - Creates tiles inside of rectangle");
		}

		ImGui::SameLine();

		ImGui::DisabledButton(ICON_FA_TOOLS "##2", TOOL_BUTTON_SIZE);

		ImGui::PopStyleVar(2);

		ImGui::SameLine(0.0f, 16.0f);

		if (auto pActiveTool = TOOL_MANAGER().GetActiveTool())
		{
			const auto& gridCoords = pActiveTool->GetGridCoords();
			ImGui::TextColored(ImVec4{ 0.7f, 1.0f, 7.0f, 1.0f },
				std::format("Grid Coords [x = {}, y = {}]", gridCoords.x, gridCoords.y).c_str());

			ImGui::SameLine(0.0f, 16.0f);

			const auto& worldCoords = pActiveTool->GetMouseWorldCoords();
			ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 1.0f, 1.0f },
				std::format("World Coords [x = {}, y = {}]", worldCoords.x, worldCoords.y).c_str());
		}

		ImGui::Separator();
		ImGui::AddSpaces(1);
	}

}
