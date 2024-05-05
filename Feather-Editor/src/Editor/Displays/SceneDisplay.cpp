#include "SceneDisplay.h"

#include "Renderer/Buffers/Framebuffer.h"

#include <imgui.h>

namespace Feather {

	SceneDisplay::SceneDisplay(Registry& registry)
		: m_Registry{ registry }
	{}

	void SceneDisplay::Draw()
	{
		static bool isOpen{ true };
		if (!ImGui::Begin("Scene", &isOpen))
		{
			ImGui::End();
			return;
		}

		if (ImGui::BeginChild("##SceneChild", ImVec2{ 0.0f, 0.0f }, NULL, ImGuiWindowFlags_NoScrollWithMouse))
		{
			const auto& fb = m_Registry.GetContext<std::shared_ptr<Framebuffer>>();

			ImGui::SetCursorPos(ImVec2{ 0.0f, 0.0f });

			ImGui::Image(
				(ImTextureID)fb->GetTextureID(),
				ImVec2{
					 static_cast<float>(fb->GetWidth()),
					 static_cast<float>(fb->GetHeight())
				},
				ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f }
			);
			ImGui::EndChild();

			// Check for resize based on window size
			ImVec2 windowSize{ ImGui::GetWindowSize() };
			if (fb->GetWidth() != static_cast<int>(windowSize.x) || fb->GetHeight() != static_cast<int>(windowSize.y))
				fb->Resize(static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
		}

		ImGui::End();
	}
}
