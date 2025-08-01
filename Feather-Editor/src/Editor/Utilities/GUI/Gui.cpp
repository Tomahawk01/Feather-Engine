#include "Gui.h"

#include "Logger/Logger.h"
#include "Windowing/Window/Window.h"
#include "Core/Resources/fonts/default_fonts.h"

#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"
#include "Editor/Utilities/Fonts/editor_fonts.h"

// IMGUI 
#include "ImGuiUtils.h"
#include "imgui.h"
#include <imgui_internal.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

#include <SDL_opengl.h>

namespace Feather {

	bool Gui::InitImGui(Window* window)
	{
		if (m_Initialized)
			return false;

		const char* glslVersion = "#version 450";
		IMGUI_CHECKVERSION();

		if (!ImGui::CreateContext())
		{
			F_ERROR("Failed to create ImGui Context");
			return false;
		}

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		io.ConfigWindowsMoveFromTitleBarOnly = true;

		ImFont* defaultFont = io.Fonts->AddFontDefault();
		ImGui::AddFont("default", defaultFont, 13.0f);

		float baseFontSize = 16.0f;
		float iconFontSize = baseFontSize * 2.0f / 3.0f;

		// merge in icons from Font Awesome
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphMinAdvanceX = iconFontSize;
		icons_config.GlyphOffset = ImVec2{ 0.0f, 2.0f };
		io.Fonts->AddFontFromMemoryTTF(g_FaSolid900, g_FaSolid900Size, baseFontSize, &icons_config, icons_ranges);

		ImGui::AddFontFromMemory("roboto-bold-14", g_RobotoBoldFont, g_RobotoBoldFontSize, 14.0f);

		ImGui::AddFontFromMemory("roboto-bold-24", g_RobotoBoldFont, g_RobotoBoldFontSize, 24.0f);

		if (!ImGui_ImplSDL2_InitForOpenGL(window->GetWindow().get(), window->GetGLContext()))
		{
			F_ERROR("Failed to intialize ImGui SDL2 for OpenGL!");
			return false;
		}

		if (!ImGui_ImplOpenGL3_Init(glslVersion))
		{
			F_ERROR("Failed to intialize ImGui OpenGL3!");
			return false;
		}

		ImGui::InitDefaultStyles();
		return true;
	}

	void Gui::BeginImGui()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
	}

	void Gui::EndImGui(Window* window)
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_Window* backupCurrentWindow = SDL_GL_GetCurrentWindow();
			SDL_GLContext backupCurrentContext = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();

			SDL_GL_MakeCurrent(backupCurrentWindow, backupCurrentContext);
		}
	}

	void Gui::ShowImGuiDemo()
	{
		ImGui::ShowDemoWindow();
	}

}
