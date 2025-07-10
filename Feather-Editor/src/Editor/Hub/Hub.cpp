#include "Hub.h"

#include "Logger/Logger.h"
#include "Utils/HelperUtilities.h"
#include "Windowing/Window/Window.h"
#include "Filesystem/Dialogs/FileDialog.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Renderer/Essentials/Texture.h"

#include "Editor/Utilities/GUI/Gui.h"
#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Loaders/ProjectLoader.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL_opengl.h>

#include <filesystem>

constexpr ImVec2 BUTTON_SIZE = ImVec2{ 100.0f, 20.0f };

namespace Feather {

	Hub::Hub(Window& window)
		: m_Window{ window }
		, m_Running{ false }
		, m_LoadError{ false }
		, m_Event{}
		, m_State{ HubState::Default }
		, m_Width{ static_cast<float>(m_Window.GetWidth()) }
		, m_Height{ static_cast<float>(m_Window.GetHeight()) }
		, m_NewProjectName{}
		, m_NewProjectPath{ DEFAULT_PROJECT_PATH }
		, m_PrevProjectPath{}
		, m_PrevProjectName{}
	{
		std::filesystem::path projectPath{ m_NewProjectPath };
		if (!std::filesystem::exists(projectPath))
		{
			std::error_code ec;
			if (!std::filesystem::create_directories(projectPath, ec))
			{
				F_ERROR("HUB - Failed to create directories. Error: {}", ec.message());
				m_State = HubState::Close;
				// TODO: Close and actually log an error
			}
		}
	}

	Hub::~Hub()
	{}

	bool Hub::Run()
	{
		m_Running = Initialize();

		while (m_Running)
		{
			ProcessEvents();
			Update();
			Render();
		}

		bool closed{ m_State == HubState::Close };

		if (!closed)
		{
			SDL_SetWindowBordered(m_Window.GetWindow().get(), SDL_TRUE);
			SDL_SetWindowResizable(m_Window.GetWindow().get(), SDL_TRUE);
			SDL_MaximizeWindow(m_Window.GetWindow().get());

			std::string title{ "Feather Editor - " };
			title += !m_NewProjectName.empty() ? m_NewProjectName : m_PrevProjectName;
			SDL_SetWindowTitle(m_Window.GetWindow().get(), title.c_str());
		}

		return !closed;
	}

	bool Hub::Initialize()
	{
		return true;
	}

	void Hub::DrawGui()
	{
		if (!ImGui::Begin("##_HudDisplay", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration))
		{
			ImGui::End();
			return;
		}

		ImGui::SetWindowPos(ImGui::GetMainViewport()->Pos);
		ImGui::SetWindowSize(ImVec2{ m_Width, m_Height });

		// Draw the Feather logo
		auto logo = ASSET_MANAGER().GetTexture("feather_logo");
		if (logo)
		{
			ImGui::SetCursorPos(ImVec2{ (m_Width - logo->GetWidth()) * 0.5f, 5.0f });
			ImGui::Image((ImTextureID)(intptr_t)logo->GetID(), ImVec2{ static_cast<float>(logo->GetWidth()), static_cast<float>(logo->GetHeight()) });
		}

		switch (m_State)
		{
		case HubState::Default: DrawDefault(); break;
		case HubState::NewProject: DrawNewProject(); break;
		case HubState::OpenProject: DrawOpenProject(); break;
		default: break;
		}

		ImGui::End();
	}

	void Hub::DrawDefault()
	{
		ImGui::SetCursorPos(ImVec2{ m_Width * 0.5f - BUTTON_SIZE.x, 450.0f });
		if (ImGui::Button("New Project", BUTTON_SIZE))
		{
			m_State = HubState::NewProject;
		}

		ImGui::SameLine();
		if (ImGui::Button("Open Project", BUTTON_SIZE))
		{
			m_State = HubState::OpenProject;
		}

		ImGui::SetCursorPos(ImVec2{ m_Width - (BUTTON_SIZE.x + 16.0f), m_Height - (BUTTON_SIZE.y + 16.0f) });
		if (ImGui::Button("Close", BUTTON_SIZE))
		{
			m_State = HubState::Close;
			m_Running = false;
		}
	}

	void Hub::DrawNewProject()
	{
		ImGui::SetCursorPosY(450.0f);
		ImGui::OffsetTextX("New Project", 32.0f);
		ImGui::SameLine();

		char newProjectBuffer[256]{ 0 };
		char newProjectPathBuffer[256]{ 0 };
#ifdef _WIN32
		strcpy_s(newProjectBuffer, m_NewProjectName.c_str());
		strcpy_s(newProjectPathBuffer, m_NewProjectPath.c_str());
#else
		strcpy(newProjectBuffer, m_NewProjectName.c_str());
		strcpy(newProjectPathBuffer, m_NewProjectPath.c_str());
#endif
		ImGui::SetCursorPosX(128.0f);
		if (ImGui::InputText("##ProjectName", newProjectBuffer, sizeof(newProjectBuffer)))
		{
			m_NewProjectName = std::string{ newProjectBuffer };
		}

		ImGui::OffsetTextX("Location", 32.0f);
		ImGui::SameLine();
		ImGui::SetCursorPosX(128.0f);
		if (ImGui::InputText("##ProjectPath", newProjectPathBuffer, sizeof(newProjectPathBuffer)))
		{
			m_NewProjectPath = std::string{ newProjectPathBuffer };
		}
		ImGui::SameLine();
		if (ImGui::Button("Browse"))
		{
			FileDialog fd{};
			const auto filepath = fd.SelectFolderDialog("Open", BASE_PATH);
			if (!filepath.empty())
			{
#ifdef _WIN32
				strcpy_s(newProjectPathBuffer, filepath.c_str());
#else
				strcpy(newProjectPathBuffer, filepath.c_str());
#endif
				m_NewProjectPath = filepath;
			}
		}

		if (!m_NewProjectName.empty() && !m_NewProjectPath.empty())
		{
			std::filesystem::path projectPath{ m_NewProjectPath };

			if (std::filesystem::exists(projectPath))
			{
				ImGui::SetCursorPosX(128.f);
				if (ImGui::Button("Create", BUTTON_SIZE))
				{
					// Create the project
					ProjectLoader pl{};
					if (!pl.CreateNewProject(m_NewProjectName, m_NewProjectPath))
					{
						// TODO: show an error
					}
					else
					{
						m_Running = false;
						m_State = HubState::CreateNew;
					}
				}
			}
		}

		ImGui::SetCursorPos(ImVec2{ m_Width - (BUTTON_SIZE.x + 16.0f), m_Height - (BUTTON_SIZE.y + 16.0f) });
		if (ImGui::Button("Cancel", BUTTON_SIZE))
		{
			m_State = HubState::Default;
			m_NewProjectName.clear();
			m_NewProjectPath = DEFAULT_PROJECT_PATH;
		}
	}

	void Hub::DrawOpenProject()
	{
		ImGui::SetCursorPos(ImVec2{ 200.0f, 450.0f });
		ImGui::Text("Project");
		ImGui::SameLine();
		char prevProjectBuffer[256]{ 0 };
#ifdef _WIN32
		strcpy_s(prevProjectBuffer, m_PrevProjectName.c_str());
#else
		strcpy(prevProjectBuffer, m_PrevProjectName.c_str());
#endif
		ImGui::PushItemWidth(200.0f);
		ImGui::SetCursorPosX(300.0f);
		if (ImGui::InputText("##PrevProjectName", prevProjectBuffer, sizeof(prevProjectBuffer)))
		{
			m_PrevProjectName = std::string{ prevProjectBuffer };
		}
		ImGui::SameLine();
		if (ImGui::Button("Browse"))
		{
			FileDialog fd{};
			const auto filepath = fd.OpenFileDialog("Open", "", { "*.fprj" });
			if (!filepath.empty())
			{
#ifdef _WIN32
				strcpy_s(prevProjectBuffer, filepath.c_str());
#else
				strcpy(prevProjectBuffer, filepath.c_str());
#endif
				m_PrevProjectPath = filepath;
				std::filesystem::path path{ filepath };
				m_PrevProjectName = path.stem().string();
			}
		}

		if (!m_PrevProjectPath.empty() && std::filesystem::exists(std::filesystem::path{ m_PrevProjectPath }))
		{
			ImGui::SetCursorPos(ImVec2{ 300.0f, 482.0f });
			if (ImGui::Button("Load Project", BUTTON_SIZE))
			{
				// Create the project
				ProjectLoader pl{};
				if (!pl.LoadProject(m_PrevProjectPath))
				{
					// TODO: show an error
				}
				else
				{
					m_Running = false;
					m_State = HubState::OpenProject;
				}
			}
		}

		ImGui::SetCursorPos(ImVec2{ m_Width - (BUTTON_SIZE.x + 16.0f), m_Height - (BUTTON_SIZE.y + 16.0f) });
		if (ImGui::Button("Cancel", BUTTON_SIZE))
		{
			m_State = HubState::Default;
			m_PrevProjectName.clear();
			m_PrevProjectPath.clear();
		}

		ImGui::PopItemWidth();
	}

	void Hub::ProcessEvents()
	{
		// Process Events
		while (SDL_PollEvent(&m_Event))
		{
			ImGui_ImplSDL2_ProcessEvent(&m_Event);
			switch (m_Event.type)
			{
			case SDL_QUIT: m_Running = false; break;
			case SDL_KEYDOWN:		  // keyboard.OnKeyPressed( m_Event.key.keysym.sym ); break;
			case SDL_KEYUP:			  // keyboard.OnKeyReleased( m_Event.key.keysym.sym ); break;
			case SDL_MOUSEBUTTONDOWN: // mouse.OnBtnPressed( m_Event.button.button ); break;
			case SDL_MOUSEBUTTONUP:	  // mouse.OnBtnReleased( m_Event.button.button ); break;
			default: break;
			}
		}
	}

	void Hub::Update()
	{
	}

	void Hub::Render()
	{
		Gui::BeginImGui();
		DrawGui();
		Gui::EndImGui(&m_Window);

		SDL_GL_SwapWindow(m_Window.GetWindow().get());
	}

}
