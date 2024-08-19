#include "Application.h"

#include <Logger/Logger.h>

#include <Renderer/Core/Renderer.h>

#include <Core/ECS/MainRegistry.h>
#include <Core/Resources/AssetManager.h>
#include <Core/CoreUtils/CoreUtilities.h>
#include <Core/CoreUtils/CoreEngineData.h>

#include <Core/Systems/ScriptingSystem.h>
#include <Core/Systems/RenderSystem.h>
#include <Core/Systems/RenderUISystem.h>
#include <Core/Systems/RenderShapeSystem.h>
#include <Core/Systems/AnimationSystem.h>
#include <Core/Systems/PhysicsSystem.h>

#include <Core/Scripting/InputManager.h>
#include <Windowing/Window/Window.h>
#include <Physics/ContactListener.h>

// IMGUI testing
#include <imgui_internal.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL_opengl.h>

// Displays
#include "Editor/Displays/SceneDisplay.h"
#include "Editor/Displays/LogDisplay.h"
#include "Editor/Displays/TilesetDisplay.h"
#include "Editor/Displays/TilemapDisplay.h"
#include "Editor/Displays/AssetDisplay.h"

#include "Editor/Utilities/EditorFramebuffers.h"
#include "Editor/Utilities/editor_textures.h"
#include "Editor/Systems/GridSystem.h"
#include "Editor/Scene/SceneManager.h"

namespace Feather {

    Application& Application::GetInstance()
    {
		static Application app{};
		return app;
    }

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
		F_INIT_LOGS(false, true);

		// TODO: Load core engine data
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

		SDL_DisplayMode displayMode;
		SDL_GetCurrentDisplayMode(0, &displayMode);

		// Create the Window
		m_Window = std::make_unique<Window>("Feather Engine", displayMode.w, displayMode.h, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, true, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MOUSE_CAPTURE | SDL_WINDOW_MAXIMIZED);

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

		auto renderer = std::make_shared<Renderer>();

		// Enable alpha blending
		renderer->SetCapability(Renderer::GLCapability::BLEND, true);
		renderer->SetBlendCapability(
			Renderer::BlendingFactors::SRC_ALPHA,
			Renderer::BlendingFactors::ONE_MINUS_SRC_ALPHA
		);

		auto& mainRegistry = MAIN_REGISTRY();
		mainRegistry.Initialize();

		if (!mainRegistry.AddToContext<std::shared_ptr<Renderer>>(renderer))
		{
			F_FATAL("Failed to add the Renderer to the registry context!");
			return false;
		}

		auto renderSystem = std::make_shared<RenderSystem>();
		if (!renderSystem)
		{
			F_FATAL("Failed to create render system!");
			return false;
		}
		if (!mainRegistry.AddToContext<std::shared_ptr<RenderSystem>>(renderSystem))
		{
			F_FATAL("Failed to add the render system to the registry context!");
			return false;
		}

		auto renderUISystem = std::make_shared<RenderUISystem>();
		if (!renderUISystem)
		{
			F_FATAL("Failed to create render UI system!");
			return false;
		}
		if (!mainRegistry.AddToContext<std::shared_ptr<RenderUISystem>>(renderUISystem))
		{
			F_FATAL("Failed to add the render UI system to the registry context!");
			return false;
		}

		auto renderShapeSystem = std::make_shared<RenderShapeSystem>();
		if (!renderShapeSystem)
		{
			F_FATAL("Failed to create render shape system!");
			return false;
		}
		if (!mainRegistry.AddToContext<std::shared_ptr<RenderShapeSystem>>(renderShapeSystem))
		{
			F_FATAL("Failed to add the render shape system to the registry context!");
			return false;
		}

		if (!InitImGui())
		{
			F_FATAL("Failed to initialize ImGui!");
			return false;
		}

		if (!LoadShaders())
		{
			F_FATAL("Failed to load shaders!");
			return false;
		}

		if (!LoadEditorTextures())
		{
			F_FATAL("Failed to load editor textures!");
			return false;
		}

		if (!CreateDisplays())
		{
			F_FATAL("Failed to create displays!");
			return false;
		}

		renderer->SetLineWidth(4.0f);

		if (!mainRegistry.GetAssetManager().CreateDefaultFonts())
		{
			F_FATAL("Failed to create default fonts!");
			return false;
		}

		auto editorFramebuffers = std::make_shared<EditorFramebuffers>();
		if (!editorFramebuffers)
		{
			F_FATAL("Failed to create editor framebuffers to context");
			return false;
		}
		if (!mainRegistry.AddToContext<std::shared_ptr<EditorFramebuffers>>(editorFramebuffers))
		{
			F_FATAL("Failed to add editor framebuffers to registry context");
			return false;
		}

		editorFramebuffers->mapFramebuffers.emplace(FramebufferType::SCENE, std::make_shared<Framebuffer>(640, 480, false));
		editorFramebuffers->mapFramebuffers.emplace(FramebufferType::TILEMAP, std::make_shared<Framebuffer>(640, 480, false));

		if (!mainRegistry.AddToContext<std::shared_ptr<GridSystem>>(std::make_shared<GridSystem>()))
		{
			F_FATAL("Failed to add grid system to registry context");
			return false;
		}

		// TODO: Remove these test scenes
		SCENE_MANAGER().AddScene("DefaultScene");
		SCENE_MANAGER().AddScene("TestScene");

		return true;
    }

    bool Application::LoadShaders()
    {
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		if (!assetManager.AddShader("basic", "assets/shaders/basicShader.vert", "assets/shaders/basicShader.frag"))
		{
			F_FATAL("Failed to add basicShader to the asset manager!");
			return false;
		}
		if (!assetManager.AddShader("color", "assets/shaders/colorShader.vert", "assets/shaders/colorShader.frag"))
		{
			F_FATAL("Failed to add colorShader to the asset manager!");
			return false;
		}
		if (!assetManager.AddShader("circle", "assets/shaders/circleShader.vert", "assets/shaders/circleShader.frag"))
		{
			F_FATAL("Failed to add the color shader to the asset manager");
			return false;
		}
		if (!assetManager.AddShader("font", "assets/shaders/fontShader.vert", "assets/shaders/fontShader.frag"))
		{
			F_FATAL("Failed to add the font shader to the asset manager");
			return false;
		}

		return true;
    }

	bool Application::LoadEditorTextures()
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		if (!assetManager.AddTextureFromMemory("play_button", play_button, sizeof(play_button) / sizeof(play_button[0])))
		{
			F_ERROR("Failed to load texture 'play_button' from memory");
			return false;
		}
		assetManager.GetTexture("play_button")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("stop_button", stop_button, sizeof(stop_button) / sizeof(stop_button[0])))
		{
			F_ERROR("Failed to load texture 'stop_button' from memory");
			return false;
		}
		assetManager.GetTexture("stop_button")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("music_icon", music_icon, sizeof(music_icon) / sizeof(music_icon[0])))
		{
			F_ERROR("Failed to load texture 'music_icon' from memory");
			return false;
		}
		assetManager.GetTexture("music_icon")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("scene_icon", scene_icon, sizeof(scene_icon) / sizeof(scene_icon[0])))
		{
			F_ERROR("Failed to load texture 'scene_icon' from memory");
			return false;
		}
		assetManager.GetTexture("scene_icon")->SetIsEditorTexture(true);

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
			ImGui_ImplSDL2_ProcessEvent(&m_Event);

			switch (m_Event.type)
			{
			case SDL_QUIT:
				m_IsRunning = false;
				break;
			case SDL_KEYDOWN:
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
			case SDL_CONTROLLERBUTTONDOWN:
				inputManager.GamepadButtonPressed(m_Event);
				break;
			case SDL_CONTROLLERBUTTONUP:
				inputManager.GamepadButtonReleased(m_Event);
				break;
			case SDL_CONTROLLERDEVICEADDED:
				inputManager.AddGamepad(m_Event.jdevice.which);
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
				inputManager.RemoveGamepad(m_Event.jdevice.which);
				break;
			case SDL_JOYAXISMOTION:
				inputManager.GamepadAxisValues(m_Event);
				break;
			case SDL_JOYHATMOTION:
				inputManager.GamepadHatValues(m_Event);
				break;
			case SDL_WINDOWEVENT:
				switch (m_Event.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					m_Window->SetSize(m_Event.window.data1, m_Event.window.data2);
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}
    }

    void Application::Update()
    {
		auto& engineData = CoreEngineData::GetInstance();
		engineData.UpdateDeltaTime();

		auto& mainRegistry = MAIN_REGISTRY();
		auto& displayHolder = mainRegistry.GetContext<std::shared_ptr<DisplayHolder>>();

		for (const auto& display : displayHolder->displays)
			display->Update();

		// Updating inputs
		auto& inputManager = InputManager::GetInstance();
		auto& keyboard = inputManager.GetKeyboard();
		keyboard.Update();
		auto& mouse = inputManager.GetMouse();
		mouse.Update();
		inputManager.UpdateGamepads();
    }

    void Application::Render()
    {
		BeginImGui();
		RenderImGui();
		EndImGui();

		SDL_GL_SwapWindow(m_Window->GetWindow().get());
    }

    void Application::CleanUp()
    {
		SDL_Quit();
    }

	bool Application::CreateDisplays()
	{
		auto& mainRegistry = MAIN_REGISTRY();

		auto displayHolder = std::make_shared<DisplayHolder>();
		if (!mainRegistry.AddToContext<std::shared_ptr<DisplayHolder>>(displayHolder))
		{
			F_ERROR("Failed to add display holder to main registry");
			return false;
		}

		auto sceneDisplay = std::make_unique<SceneDisplay>();
		if (!sceneDisplay)
		{
			F_ERROR("Failed to create a Scene Display");
			return false;
		}

		auto logDisplay = std::make_unique<LogDisplay>();
		if (!logDisplay)
		{
			F_ERROR("Failed to create a Log Display");
			return false;
		}

		auto tilesetDisplay = std::make_unique<TilesetDisplay>();
		if (!tilesetDisplay)
		{
			F_ERROR("Failed to create a Tileset Display");
			return false;
		}

		auto tilemapDisplay = std::make_unique<TilemapDisplay>();
		if (!tilemapDisplay)
		{
			F_ERROR("Failed to create a Tilemap Display");
			return false;
		}

		auto assetDisplay = std::make_unique<AssetDisplay>();
		if (!assetDisplay)
		{
			F_ERROR("Failed to create a Asset Display");
			return false;
		}

		displayHolder->displays.push_back(std::move(sceneDisplay));
		displayHolder->displays.push_back(std::move(logDisplay));
		displayHolder->displays.push_back(std::move(tilesetDisplay));
		displayHolder->displays.push_back(std::move(tilemapDisplay));
		displayHolder->displays.push_back(std::move(assetDisplay));

		return true;
	}

	bool Application::InitImGui()
	{
		const char* glslVersion = "#version 450 core";
		IMGUI_CHECKVERSION();

		if (!ImGui::CreateContext())
		{
			F_FATAL("Failed to create ImGui context!");
			return false;
		}

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		io.ConfigWindowsMoveFromTitleBarOnly = true;

		if (!ImGui_ImplSDL2_InitForOpenGL(m_Window->GetWindow().get(), m_Window->GetGLContext()))
		{
			F_FATAL("Failed to initialize ImGui SDL2 for OpenGL!");
			return false;
		}

		if (!ImGui_ImplOpenGL3_Init(glslVersion))
		{
			F_FATAL("Failed to initialize ImGui OpenGL3!");
			return false;
		}

		return true;
	}

	void Application::BeginImGui()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
	}

	void Application::EndImGui()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_GLContext backupContext = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();

			SDL_GL_MakeCurrent(m_Window->GetWindow().get(), backupContext);
		}
	}

	void Application::RenderImGui()
	{
		const auto dockSpaceId = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		if (static auto firstTime = true; firstTime) [[unlikely]]
		{
			firstTime = false;

			ImGui::DockBuilderRemoveNode(dockSpaceId);
			ImGui::DockBuilderAddNode(dockSpaceId);

			auto centerNodeId = dockSpaceId;
			const auto leftNodeId = ImGui::DockBuilderSplitNode(centerNodeId, ImGuiDir_Left, 0.2f, nullptr, &centerNodeId);
			const auto rightNodeId = ImGui::DockBuilderSplitNode(centerNodeId, ImGuiDir_Right, 0.25f, nullptr, &centerNodeId);
			const auto logNodeId = ImGui::DockBuilderSplitNode(centerNodeId, ImGuiDir_Down, 0.25f, nullptr, &centerNodeId);

			ImGui::DockBuilderDockWindow("Tileset", rightNodeId);
			ImGui::DockBuilderDockWindow("Dear ImGui Demo", leftNodeId);
			ImGui::DockBuilderDockWindow("Scene", centerNodeId);
			ImGui::DockBuilderDockWindow("Tilemap Editor", centerNodeId);
			ImGui::DockBuilderDockWindow("Logs", logNodeId);
			ImGui::DockBuilderDockWindow("Assets", logNodeId);

			ImGui::DockBuilderFinish(dockSpaceId);
		}

		auto& mainRegistry = MAIN_REGISTRY();
		auto& displayHolder = mainRegistry.GetContext<std::shared_ptr<DisplayHolder>>();

		for (const auto& display : displayHolder->displays)
			display->Draw();

		ImGui::ShowDemoWindow();
	}

    Application::Application()
        : m_Window{ nullptr }, m_Event{}, m_IsRunning{ true }
    {}

}
