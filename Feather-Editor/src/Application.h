#pragma once
#include <SDL.h>

#include <memory>

namespace Feather {
	
	class Window;

	class Application
	{
	public:
		static Application& GetInstance();

		void Run();

	private:
		bool Initialize();
		bool LoadShaders();
		bool LoadEditorTextures();

		void ProcessEvents();
		void Update();
		void Render();

		void CleanUp();

		bool CreateDisplays();
		bool InitImGui();
		void BeginImGui();
		void EndImGui();
		void RenderImGui();

		void RegisterEditorMetaFunctions();

		Application();
		~Application() = default;
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

	private:
		std::unique_ptr<Feather::Window> m_Window;

		SDL_Event m_Event;
		bool m_IsRunning;
	};

}
