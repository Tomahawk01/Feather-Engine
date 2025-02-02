#pragma once

#include <SDL.h>
#include <string>

namespace Feather {

	class Window;

	enum class HubState
	{
		Default,
		NewProject,
		CreateNew,
		OpenProject,
		Close,

		NoState
	};

	class Hub
	{
	public:
		Hub(Window& window);
		~Hub();

		bool Run();

	private:
		bool Initialize();
		void DrawGui();

		void DrawDefault();
		void DrawNewProject();
		void DrawOpenProject();

		void ProcessEvents();
		void Update();
		void Render();

	private:
		Window& m_Window;
		bool m_Running;
		bool m_LoadError;
		SDL_Event m_Event;
		HubState m_State;

		float m_Width;
		float m_Height;

		std::string m_NewProjectName;
		std::string m_NewProjectPath;
		std::string m_PrevProjectPath;
		std::string m_PrevProjectName;
	};

}
