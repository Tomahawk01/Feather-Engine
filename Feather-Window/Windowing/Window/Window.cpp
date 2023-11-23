#include "Window.h"

#include <iostream>

namespace Feather {

	Window::Window(const std::string title, int width, int height, int x_pos, int y_pos, bool v_sync, Uint32 flags)
		: m_pWindow{ nullptr }, m_GLContext{}, m_Title{ title }, m_Width{ width }, m_Height{ height }, m_XPos{ x_pos }, m_YPos{ y_pos }, m_WindowFlags{ flags }
	{
		CreateNewWindow(flags);

		// Enable V-Sync
		if (v_sync)
		{
			if (!SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1"))
				std::cout << "Failed to enable V-Sync!\n";
		}
		std::cout << "Window Created Successfully\n";
	}

	Window::~Window()
	{
	}

	void Window::SetWindowName(const std::string& name)
	{
		m_Title = name;
		SDL_SetWindowTitle(m_pWindow.get(), name.c_str());
	}

	void Window::CreateNewWindow(Uint32 flags)
	{
		m_pWindow = WindowPtr(SDL_CreateWindow(m_Title.c_str(), m_XPos, m_YPos, m_Width, m_Height, flags));

		if (!m_pWindow)
		{
			std::string error = SDL_GetError();
			std::cout << "Failed to create the Window: " << error << "\n";
		}
	}

}
