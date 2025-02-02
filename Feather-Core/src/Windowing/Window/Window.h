#pragma once

#include "Utils/SDL_Wrappers.h"

#include <string>

namespace Feather {

	class Window
	{
	public:
		Window();
		Window(const std::string title, int width, int height, int x_pos, int y_pos, bool v_sync = true,
			   Uint32 flags = { SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MOUSE_CAPTURE });
		~Window() = default;

		void SetPosition(int x, int y);
		void SetSize(int width, int height);

		inline void SetGLContext(SDL_GLContext gl_context) { m_GLContext = gl_context; }
		inline SDL_GLContext& GetGLContext() { return m_GLContext; }
		inline WindowPtr& GetWindow() { return m_Window; }
		inline const std::string& GetWindowName() const { return m_Title; }

		inline const int GetXPos() const { return m_XPos; }
		inline const int SetXPos(int x_pos) { m_XPos = x_pos; }
		inline const int GetYPos() const { return m_YPos; }
		inline const int SetYPos(int y_pos) { m_YPos = y_pos; }

		inline const int GetWidth() const { return m_Width; }
		inline const int GetHeight() const { return m_Height; }

		void SetWindowName(const std::string& name);

	private:
		WindowPtr m_Window;
		SDL_GLContext m_GLContext;
		std::string m_Title;
		int m_Width;
		int m_Height;
		int m_XPos;
		int m_YPos;
		Uint32 m_WindowFlags;
	private:
		void CreateNewWindow(Uint32 flags);
	};

}
