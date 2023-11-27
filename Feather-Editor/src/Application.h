#pragma once

#include <glad/glad.h>

#include <Windowing/Window/Window.h>
#include <Core/ECS/Registry.h>

namespace Feather {
	
	class Application
	{
	public:
		static Application& GetInstance();
		~Application();

		void Run();

	private:
		std::unique_ptr<Feather::Window> m_Window;
		std::unique_ptr<Feather::Registry> m_Registry;

		SDL_Event m_Event;
		bool m_IsRunning;

	private:
		bool Initialize();
		bool LoadShaders();

		void ProcessEvents();
		void Update();
		void Render();

		void CleanUp();

		Application();
	};

}
