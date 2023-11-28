#include "Application.h"

#include <Logger/Logger.h>
#include <Renderer/Essentials/ShaderLoader.h>
#include <Renderer/Essentials/TextureLoader.h>
#include <Renderer/Essentials/Vertex.h>
#include <Renderer/Core/Camera2D.h>
#include <Core/ECS/Entity.h>
#include <Core/ECS/Components/Identification.h>
#include <Core/ECS/Components/SpriteComponent.h>
#include <Core/ECS/Components/TransformComponent.h>
#include <Core/Resources/AssetManager.h>
#include <Core/Systems/ScriptingSystem.h>
#include <Core/Systems/RenderSystem.h>
#include <Core/Systems/AnimationSystem.h>
#include <Core/Scripting/InputManager.h>

namespace Feather {

    Application& Application::GetInstance()
    {
		static Application app{};
		return app;
    }

    Application::~Application()
    {}

    void Application::Run()
    {
		if (!Initialize())
		{
			F_FATAL("Initialization failed!");
			return;
		}

		while (m_IsRunning)
		{
			ProcessEvents();
			Update();
			Render();
		}

		CleanUp();
    }

    bool Application::Initialize()
    {
		Feather::Log::Init();

		// Init SDL
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		{
			std::string error = SDL_GetError();
			F_FATAL("Failed to initialize SDL: {0}", error);
			return false;
		}

		// Setup OpenGL
		if (SDL_GL_LoadLibrary(NULL) != 0)
		{
			std::string error = SDL_GetError();
			F_FATAL("Failed to load OpenGL library: {0}", error);
			return false;
		}

		// Set OpenGL attributes
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		// Set the number of bits per channel
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

		// Create the Window
		m_Window = std::make_unique<Feather::Window>("Test Window", 640, 480, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, true, SDL_WINDOW_OPENGL);

		if (!m_Window->GetWindow())
		{
			F_FATAL("Failed to create the window!");
			return false;
		}

		// Create OpenGL context
		m_Window->SetGLContext(SDL_GL_CreateContext(m_Window->GetWindow().get()));
		if (!m_Window->GetGLContext())
		{
			std::string error = SDL_GetError();
			F_FATAL("Failed to create OpenGL context: {0}", error);
			return false;
		}

		SDL_GL_MakeCurrent(m_Window->GetWindow().get(), m_Window->GetGLContext());
		SDL_GL_SetSwapInterval(1);

		// Initialize Glad
		if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
		{
			F_FATAL("Failed to initialize Glad!");
			return false;
		}

		// Enable alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		auto assetManager = std::make_shared<Feather::AssetManager>();
		if (!assetManager)
		{
			F_FATAL("Failed to create the asset manager!");
			return false;
		}

		if (!assetManager->AddTexure("gem", "assets/textures/Gem.png", true))
		{
			F_FATAL("Failed to create and add texture!");
			return false;
		}
		if (!assetManager->AddTexure("gem_animation", "assets/textures/Gem_SpriteSheet.png", true))
		{
			F_FATAL("Failed to create and add texture!");
			return false;
		}

		// Create new test entity
		m_Registry = std::make_unique<Feather::Registry>();

		// Create Lua state
		auto lua = std::make_shared<sol::state>();
		if (!lua)
		{
			F_FATAL("Failed to create Lua state!");
			return false;
		}

		lua->open_libraries(sol::lib::base,
							sol::lib::math,
							sol::lib::os,
							sol::lib::table,
							sol::lib::io,
							sol::lib::string);

		if (!m_Registry->AddToContext<std::shared_ptr<sol::state>>(lua))
		{
			F_FATAL("Failed to add the sol::state to the registry context!");
			return false;
		}

		auto scriptSystem = std::make_shared<Feather::ScriptingSystem>(*m_Registry);
		if (!scriptSystem)
		{
			F_FATAL("Failed to create script system!");
			return false;
		}
		if (!m_Registry->AddToContext<std::shared_ptr<Feather::ScriptingSystem>>(scriptSystem))
		{
			F_FATAL("Failed to add the script system to the registry context!");
			return false;
		}

		auto renderSystem = std::make_shared<Feather::RenderSystem>(*m_Registry);
		if (!renderSystem)
		{
			F_FATAL("Failed to create render system!");
			return false;
		}
		if (!m_Registry->AddToContext<std::shared_ptr<Feather::RenderSystem>>(renderSystem))
		{
			F_FATAL("Failed to add the render system to the registry context!");
			return false;
		}

		auto animationSystem = std::make_shared<Feather::AnimationSystem>(*m_Registry);
		if (!animationSystem)
		{
			F_FATAL("Failed to create animation system!");
			return false;
		}
		if (!m_Registry->AddToContext<std::shared_ptr<Feather::AnimationSystem>>(animationSystem))
		{
			F_FATAL("Failed to add the animation system to the registry context!");
			return false;
		}

		// Camera creation
		auto camera = std::make_shared<Feather::Camera2D>();

		if (!m_Registry->AddToContext<std::shared_ptr<Feather::AssetManager>>(assetManager))
		{
			F_FATAL("Failed to add asset manager to the registry context!");
			return false;
		}

		if (!m_Registry->AddToContext<std::shared_ptr<Feather::Camera2D>>(camera))
		{
			F_FATAL("Failed to add camera to the registry context!");
			return false;
		}

		if (!LoadShaders())
		{
			F_FATAL("Failed to load shaders!");
			return false;
		}

		Feather::ScriptingSystem::RegisterLuaBindings(*lua, *m_Registry);

		if (!scriptSystem->LoadMainScript(*lua))
		{
			F_FATAL("Failed to load main lua script");
			return false;
		}

		return true;
    }

    bool Application::LoadShaders()
    {
		auto& assetManager = m_Registry->GetContext<std::shared_ptr<Feather::AssetManager>>();
		if (!assetManager)
		{
			F_FATAL("Failed to get asset shader from the registry context!");
			return false;
		}
		if (!assetManager->AddShader("basic", "assets/shaders/basicShader.vert", "assets/shaders/basicShader.frag"))
		{
			F_FATAL("Failed to add shader to the asset manager!");
			return false;
		}

		return true;
    }

    void Application::ProcessEvents()
    {
		auto& inputManager = InputManager::GetInstance();
		auto& keyboard = inputManager.GetKeyboard();
		auto& mouse = inputManager.GetMouse();

		// Process Events
		while (SDL_PollEvent(&m_Event))
		{
			switch (m_Event.type)
			{
			case SDL_QUIT:
				m_IsRunning = false;
				break;
			case SDL_KEYDOWN:
				if (m_Event.key.keysym.sym == SDLK_ESCAPE)
					m_IsRunning = false;
				keyboard.OnKeyPressed(m_Event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				keyboard.OnKeyReleased(m_Event.key.keysym.sym);
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouse.OnButtonPressed(m_Event.button.button);
				break;
			case SDL_MOUSEBUTTONUP:
				mouse.OnButtonReleased(m_Event.button.button);
				break;
			case SDL_MOUSEWHEEL:
				mouse.SetMouseWheelX(m_Event.wheel.x);
				mouse.SetMouseWheelY(m_Event.wheel.y);
				break;
			case SDL_MOUSEMOTION:
				mouse.SetMouseMoving(true);
				break;
			default:
				break;
			}
		}
    }

    void Application::Update()
    {
		auto& camera = m_Registry->GetContext<std::shared_ptr<Feather::Camera2D>>();
		if (!camera)
		{
			F_FATAL("Failed to get the camera from the registry context!");
			return;
		}
		camera->Update();

		auto& scriptSystem = m_Registry->GetContext<std::shared_ptr<Feather::ScriptingSystem>>();
		scriptSystem->Update();

		auto& animationSystem = m_Registry->GetContext<std::shared_ptr<Feather::AnimationSystem>>();
		animationSystem->Update();

		// Updating inputs
		auto& inputManager = InputManager::GetInstance();
		auto& keyboard = inputManager.GetKeyboard();
		keyboard.Update();
		auto& mouse = inputManager.GetMouse();
		mouse.Update();
    }

    void Application::Render()
    {
		auto& renderSystem = m_Registry->GetContext<std::shared_ptr<Feather::RenderSystem>>();

		glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		auto& scriptSystem = m_Registry->GetContext<std::shared_ptr<Feather::ScriptingSystem>>();
		scriptSystem->Render();
		renderSystem->Update();

		SDL_GL_SwapWindow(m_Window->GetWindow().get());
    }

    void Application::CleanUp()
    {
		SDL_Quit();
    }

    Application::Application()
        : m_Window{ nullptr }, m_Registry{ nullptr }, m_Event{}, m_IsRunning{ true }
    {}

}
