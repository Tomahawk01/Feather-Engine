#pragma once
#include <SDL.h>

#include <memory>

namespace Feather {
	
	struct CloseEditorEvent;
	class Window;

	class Application
	{
	public:
		static Application& GetInstance();

		void Run();

	private:
		bool Initialize();
		bool InitApp();
		bool LoadShaders();
		bool LoadEditorTextures();

		void ProcessEvents();
		void Update();
		void UpdateInputs();
		void Render();

		void CleanUp();

		bool CreateDisplays();
		void InitDisplays();
		void RenderDisplays();

		void RegisterEditorMetaFunctions();

		void OnCloseEditor(CloseEditorEvent& close);

		Application();
		~Application() = default;
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

	private:
		std::unique_ptr<Feather::Window> m_Window;
		std::unique_ptr<class Hub> m_Hub;

		SDL_Event m_Event;
		bool m_IsRunning;
	};

}
