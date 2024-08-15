#include "TilemapDisplay.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Core/Renderer.h"
#include "Logger/Logger.h"

#include "../Systems/GridSystem.h"
#include "../Utilities/EditorFramebuffers.h"

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

			ImGui::Image((ImTextureID)fb->GetTextureID(), imageSize, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });

			ImGui::EndChild();
		}

		ImGui::End();
	}

	void TilemapDisplay::Update()
	{
		m_TilemapCam->Update();
	}

	void TilemapDisplay::RenderTilemap()
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& editorFramebuffers = mainRegistry.GetContext<std::shared_ptr<EditorFramebuffers>>();
		auto& renderer = mainRegistry.GetContext<std::shared_ptr<Renderer>>();

		/*auto& renderSystem = mainRegistry.GetContext<std::shared_ptr<RenderSystem>>();
		auto& renderUISystem = mainRegistry.GetContext<std::shared_ptr<RenderUISystem>>();
		auto& renderShapeSystem = mainRegistry.GetContext<std::shared_ptr<RenderShapeSystem>>();*/

		const auto& fb = editorFramebuffers->mapFramebuffers[FramebufferType::TILEMAP];

		fb->Bind();
		renderer->SetViewport(0, 0, fb->GetWidth(), fb->GetHeight());
		renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		renderer->ClearBuffers(true, true, false);

		auto& gridSystem = mainRegistry.GetContext<std::shared_ptr<GridSystem>>();
		gridSystem->Update(*m_TilemapCam);
		/*renderSystem->Update();
		renderShapeSystem->Update();
		renderUISystem->Update(m_Registry.GetRegistry());*/
		fb->Unbind();
		fb->CheckResize();

		renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		renderer->ClearBuffers(true, true, false);
	}

}
