#include "Application.h"

#include <Logger/Logger.h>
#include <Logger/CrashLogger.h>
#include <Renderer/Core/Renderer.h>
#include <Renderer/Essentials/PickingTexture.h>

#include <Core/ECS/MainRegistry.h>
#include <Core/Resources/AssetManager.h>
#include <Core/CoreUtils/CoreUtilities.h>
#include <Core/CoreUtils/CoreEngineData.h>
#include <Core/CoreUtils/EngineShaders.h>
#include <Core/CoreUtils/ProjectInfo.h>
#include <Core/Events/EventDispatcher.h>
#include <Core/Events/EngineEventTypes.h>
#include <Core/Scripting/InputManager.h>

#include <Windowing/Window/Window.h>
#include <Windowing/Input/Mouse.h>
#include <Windowing/Input/Keyboard.h>
#include <Physics/ContactListener.h>

#include <Utils/ThreadPool.h>
#include <Utils/HelperUtilities.h>

// Displays
#include "Editor/Displays/MenuDisplay.h"
#include "Editor/Displays/SceneDisplay.h"
#include "Editor/Displays/ScriptDisplay.h"
#include "Editor/Displays/SceneHierarchyDisplay.h"
#include "Editor/Displays/AssetDisplay.h"
#include "Editor/Displays/TileDetailsDisplay.h"
#include "Editor/Displays/LogDisplay.h"
#include "Editor/Displays/EditorStyleToolDisplay.h"
#include "Editor/Displays/TilesetDisplay.h"
#include "Editor/Displays/TilemapDisplay.h"
#include "Editor/Displays/ContentDisplay.h"
#include "Editor/Displays/PackageDisplay.h"
#include "Editor/Displays/ProjectSettingsDisplay.h"

#include "Editor/Utilities/editor_textures.h"
#include "Editor/Utilities/EditorState.h"
#include "Editor/Utilities/EditorFramebuffers.h"
#include "Editor/Utilities/DrawComponentUtils.h"
#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"

#include "Editor/Systems/GridSystem.h"
#include "Editor/Systems/EditorRenderSystem.h"
#include "Editor/Scene/SceneManager.h"
#include "Editor/Events/EditorEventTypes.h"
#include "Editor/Hub/Hub.h"

// IMGUI
#include <Editor/Utilities/GUI/Gui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL_opengl.h>

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

		if (!m_Hub || !m_Hub->Run())
			return;

		InitApp();

		while (m_IsRunning)
		{
			ProcessEvents();
			Update();
			Render();
			UpdateInputs();
			SCENE_MANAGER().UpdateScenes();
		}

		CleanUp();
    }

    bool Application::Initialize()
    {
#if DEBUG
		F_INIT_LOGS(true, true);
#else
		F_INIT_LOGS(false, true);
#endif

		FEATHER_INIT_CRASH_LOGS();

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
		m_Window = std::make_unique<Window>("Feather Engine", 800, 600, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, true, SDL_WINDOW_OPENGL);
		/*
		 * SDL Hack - If we create the window as borderless, we lose the icon in the title bar.
		 * If we create the window with a border, then hide the border.
		 * When we show the border later on, the icon will be there
		 */
		SDL_SetWindowBordered(m_Window->GetWindow().get(), SDL_FALSE);

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

		auto& mainRegistry = MAIN_REGISTRY();
		if (!mainRegistry.Initialize(true))
		{
			F_FATAL("Failed to initialize the main registry!");
			return false;
		}

		if (!Gui::InitImGui(m_Window.get()))
		{
			F_FATAL("Failed to initialize ImGui!");
			return false;
		}

		if (!LoadEditorTextures())
		{
			F_FATAL("Failed to load editor textures!");
			return false;
		}

		mainRegistry.AddToContext<ProjectInfoPtr>(std::make_shared<ProjectInfo>());
		mainRegistry.AddToContext<EditorStatePtr>(std::make_shared<EditorState>());
		m_Hub = std::make_unique<Hub>(*m_Window);

		return true;
	}

	bool Application::InitApp()
	{
		if (!LoadShaders())
		{
			F_FATAL("Failed to load shaders!");
			return false;
		}

		if (!CreateDisplays())
		{
			F_FATAL("Failed to create displays!");
			return false;
		}

		if (!ASSET_MANAGER().CreateDefaultFonts())
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
		if (!MAIN_REGISTRY().AddToContext<std::shared_ptr<EditorFramebuffers>>(editorFramebuffers))
		{
			F_FATAL("Failed to add editor framebuffers to registry context");
			return false;
		}

		editorFramebuffers->mapFramebuffers.emplace(FramebufferType::SCENE, std::make_shared<Framebuffer>(640, 480, false));
		editorFramebuffers->mapFramebuffers.emplace(FramebufferType::TILEMAP, std::make_shared<Framebuffer>(640, 480, false));

		if (!MAIN_REGISTRY().AddToContext<std::shared_ptr<GridSystem>>(std::make_shared<GridSystem>()))
		{
			F_FATAL("Failed to add grid system to registry context");
			return false;
		}

		if (!MAIN_REGISTRY().AddToContext<EditorRenderSystemPtr>(std::make_shared<EditorRenderSystem>()))
		{
			F_FATAL("Failed add the editor render system registry context");
			return false;
		}

		auto pickingTexture = std::make_shared<PickingTexture>(640, 480);
		if (!pickingTexture)
		{
			F_ERROR("Failed to create the picking texture");
			return false;
		}
		if (!MAIN_REGISTRY().AddToContext<std::shared_ptr<PickingTexture>>(pickingTexture))
		{
			F_ERROR("Failed to add the picking texture to the registry context");
			return false;
		}

		ADD_EVENT_HANDLER(CloseEditorEvent, &Application::OnCloseEditor, *this);

		// Register meta fuctions
		RegisterEditorMetaFunctions();
		CoreEngineData::RegisterMetaFunctions();

		// Set the Crash Logger path to the running project
		const auto& projectPath = CORE_GLOBALS().GetProjectPath();
		auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
		FEATHER_CRASH_LOGGER().SetProjectPath(projectInfo->GetProjectPath().string());

		MAIN_REGISTRY().AddToContext<SharedThreadPool>(std::make_shared<ThreadPool>(6));

		return true;
    }

    bool Application::LoadShaders()
    {
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		if (!assetManager.AddShaderFromMemory("basic", basicShaderVert, basicShaderFrag))
		{
			F_FATAL("Failed to add basicShader to the asset manager!");
			return false;
		}
		if (!assetManager.AddShaderFromMemory("color", colorShaderVert, colorShaderFrag))
		{
			F_FATAL("Failed to add colorShader to the asset manager!");
			return false;
		}
		if (!assetManager.AddShaderFromMemory("circle", circleShaderVert, circleShaderFrag))
		{
			F_FATAL("Failed to add the color shader to the asset manager");
			return false;
		}
		if (!assetManager.AddShaderFromMemory("font", fontShaderVert, fontShaderFrag))
		{
			F_FATAL("Failed to add the font shader to the asset manager");
			return false;
		}
		if (!assetManager.AddShaderFromMemory("picking", pickingShaderVert, pickingShaderFrag))
		{
			F_ERROR("Failed to add the picking shader to the asset manager");
			return false;
		}

		return true;
    }

	bool Application::LoadEditorTextures()
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		if (!assetManager.AddTextureFromMemory("play_button", g_PlayButton, g_PlayButtonSize))
		{
			F_ERROR("Failed to load texture 'play_button' from memory");
			return false;
		}
		assetManager.GetTexture("play_button")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("stop_button", g_StopButton, g_StopButtonSize))
		{
			F_ERROR("Failed to load texture 'stop_button' from memory");
			return false;
		}
		assetManager.GetTexture("stop_button")->SetIsEditorTexture(true);

		// ============= Gizmo Textures =============
		if (!assetManager.AddTextureFromMemory("x_axis_translate", g_XAxisArrow, g_XAxisArrowSize))
		{
			F_ERROR("Failed to load texture 'x_axis_translate' from memory");
			return false;
		}
		assetManager.GetTexture("x_axis_translate")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("y_axis_translate", g_YAxisArrow, g_YAxisArrowSize))
		{
			F_ERROR("Failed to load texture 'y_axis_translate' from memory");
			return false;
		}
		assetManager.GetTexture("y_axis_translate")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("x_axis_scale", g_XAxisScale, g_XAxisScaleSize))
		{
			F_ERROR("Failed to load texture 'x_axis_scale' from memory");
			return false;
		}
		assetManager.GetTexture("x_axis_scale")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("y_axis_scale", g_YAxisScale, g_YAxisScaleSize))
		{
			F_ERROR("Failed to load texture 'y_axis_scale' from memory");
			return false;
		}
		assetManager.GetTexture("y_axis_scale")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("rotate_tool", g_RotateTool, g_RotateToolSize))
		{
			F_ERROR("Failed to load texture 'rotate_tool' from memory");
			return false;
		}
		assetManager.GetTexture("rotate_tool")->SetIsEditorTexture(true);

		// ============= Content Display Textures =============
		if (!assetManager.AddTextureFromMemory("file_icon", g_FileIcon, g_FileIconSize))
		{
			F_ERROR("Failed to load texture 'file_icon' from memory");
			return false;
		}
		assetManager.GetTexture("file_icon")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("folder_icon", g_FolderIcon, g_FolderIconSize))
		{
			F_ERROR("Failed to load texture 'folder_icon' from memory");
			return false;
		}
		assetManager.GetTexture("folder_icon")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("music_icon", g_MusicIcon, g_MusicIconSize))
		{
			F_ERROR("Failed to load texture 'music_icon' from memory");
			return false;
		}
		assetManager.GetTexture("music_icon")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("image_icon", g_ImageIcon, g_ImageIconSize))
		{
			F_ERROR("Failed to load texture 'image_icon' from memory");
			return false;
		}
		assetManager.GetTexture("image_icon")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("scene_icon", g_SceneIcon, g_SceneIconSize))
		{
			F_ERROR("Failed to load texture 'scene_icon' from memory");
			return false;
		}
		assetManager.GetTexture("scene_icon")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("feather_logo", g_FeatherLogo, g_FeatherLogoSize))
		{
			F_ERROR("Failed to load texture 'feather_logo' from memory");
			return false;
		}
		assetManager.GetTexture("feather_logo")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("ZZ_F_PlayerStart", g_PlayerStart, g_PlayerStartSize))
		{
			F_ERROR("Failed to load texture 'ZZ_F_PlayerStart' from memory");
			return false;
		}
		assetManager.GetTexture("ZZ_F_PlayerStart")->SetIsEditorTexture(true);

		if (!assetManager.AddTextureFromMemory("ZZ_F_default_player", g_DefaultPlayer, g_DefaultPlayerSize))
		{
			F_ERROR("Failed to load texture 'ZZ_F_default_player' from memory");
			return false;
		}
		assetManager.GetTexture("ZZ_F_default_player")->SetIsEditorTexture(true);

		if (!assetManager.AddCursorFromMemory("ZZ_F_PanningCursor", g_PanningCursor, g_PanningCursorSize))
		{
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
				keyboard.OnKeyPressed(m_Event.key.keysym.sym);
				EVENT_DISPATCHER().EmitEvent(KeyEvent{ .key = m_Event.key.keysym.sym, .type = EKeyEventType::Pressed });
				break;
			case SDL_KEYUP:
				keyboard.OnKeyReleased(m_Event.key.keysym.sym);
				EVENT_DISPATCHER().EmitEvent(KeyEvent{ .key = m_Event.key.keysym.sym, .type = EKeyEventType::Released });
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
			{
				int index = inputManager.AddGamepad(m_Event.jdevice.which);
				if (index > 0)
				{
					EVENT_DISPATCHER().EmitEvent(GamepadConnectEvent{ .connectType = GamepadConnectType::Connected, .index = index });
				}

				break;
			}
			case SDL_CONTROLLERDEVICEREMOVED:
			{
				int index = inputManager.RemoveGamepad(m_Event.jdevice.which);

				if (index > 0)
				{
					EVENT_DISPATCHER().EmitEvent(GamepadConnectEvent{ .connectType = GamepadConnectType::Disconnected, .index = index });
				}

				break;
			}
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
					if (SDL_GetWindowID(m_Window->GetWindow().get()) == m_Event.window.windowID)
					{
						m_Window->SetSize(m_Event.window.data1, m_Event.window.data2);
					}
					break;
				default:
					break;
				}
				break;
			case SDL_DROPFILE:
			{
				EVENT_DISPATCHER().EmitEvent(FileEvent{
					.action = FileAction::FileDropped,
					.filepath = std::string{m_Event.drop.file}
					});
			} break;
			default:
				break;
			}

			// Process ImGui events after other events
			ImGui_ImplSDL2_ProcessEvent(&m_Event);
		}
    }

	void Application::Update()
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& displayHolder = mainRegistry.GetContext<std::shared_ptr<DisplayHolder>>();

		for (const auto& display : displayHolder->displays)
		{
			display->Update();
		}

		mainRegistry.GetAssetManager().Update();
	}

	void Application::UpdateInputs()
	{
		auto& inputManager = InputManager::GetInstance();
		auto& keyboard = inputManager.GetKeyboard();
		keyboard.Update();
		auto& mouse = inputManager.GetMouse();
		mouse.Update();
		inputManager.UpdateGamepads();
    }

    void Application::Render()
    {
		Gui::BeginImGui();
		RenderDisplays();
		Gui::EndImGui(m_Window.get());

		SDL_GL_SwapWindow(m_Window->GetWindow().get());
    }

    void Application::CleanUp()
    {
		// Save the editor state
		auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
		auto& editorState = MAIN_REGISTRY().GetContext<EditorStatePtr>();
		editorState->Save(*projectInfo);

		SDL_GL_DeleteContext(m_Window->GetGLContext());
		SDL_DestroyWindow(m_Window->GetWindow().get());

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

		auto menuDisplay = std::make_unique<MenuDisplay>();
		if (!menuDisplay)
		{
			F_ERROR("Failed to create a Menu Display");
			return false;
		}

		auto sceneDisplay = std::make_unique<SceneDisplay>();
		if (!sceneDisplay)
		{
			F_ERROR("Failed to create a Scene Display");
			return false;
		}

		auto sceneHierarchyDisplay = std::make_unique<SceneHierarchyDisplay>();
		if (!sceneHierarchyDisplay)
		{
			F_ERROR("Failed to create a Scene Hierarchy Display");
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

		auto tileDetailsDisplay = std::make_unique<TileDetailsDisplay>();
		if (!tileDetailsDisplay)
		{
			F_ERROR("Failed to create a Tile details Display");
			return false;
		}

		auto assetDisplay = std::make_unique<AssetDisplay>();
		if (!assetDisplay)
		{
			F_ERROR("Failed to create a Asset Display");
			return false;
		}

		auto contentDisplay = std::make_unique<ContentDisplay>();
		if (!contentDisplay)
		{
			F_ERROR("Failed to create a Content Display");
			return false;
		}

		auto scriptDisplay = std::make_unique<ScriptDisplay>();
		if (!scriptDisplay)
		{
			F_ERROR("Failed to create a Script Display");
			return false;
		}

		auto packageDisplay = std::make_unique<PackageGameDisplay>();
		if (!packageDisplay)
		{
			F_ERROR("Failed to create a Package Game Display");
			return false;
		}

		displayHolder->displays.push_back(std::move(menuDisplay));
		displayHolder->displays.push_back(std::move(sceneDisplay));
		displayHolder->displays.push_back(std::move(sceneHierarchyDisplay));
		displayHolder->displays.push_back(std::move(logDisplay));
		displayHolder->displays.push_back(std::move(tilesetDisplay));
		displayHolder->displays.push_back(std::move(tileDetailsDisplay));
		displayHolder->displays.push_back(std::move(tilemapDisplay));
		displayHolder->displays.push_back(std::move(assetDisplay));
		displayHolder->displays.push_back(std::move(contentDisplay));
		displayHolder->displays.push_back(std::move(scriptDisplay));
		displayHolder->displays.push_back(std::move(packageDisplay));
		displayHolder->displays.push_back(std::make_unique<ProjectSettingsDisplay>());

		return true;
	}

	void Application::InitDisplays()
	{
		const auto dockSpaceId = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		if (static auto firstTime = true; firstTime) [[unlikely]]
		{
			firstTime = false;

			ImGui::DockBuilderRemoveNode(dockSpaceId);
			ImGui::DockBuilderAddNode(dockSpaceId);

			auto centerNodeId = dockSpaceId;
			const auto leftNodeId = ImGui::DockBuilderSplitNode(centerNodeId, ImGuiDir_Left, 0.2f, nullptr, &centerNodeId);
			auto rightNodeId = ImGui::DockBuilderSplitNode(centerNodeId, ImGuiDir_Right, 0.3f, nullptr, &centerNodeId);
			const auto logNodeId = ImGui::DockBuilderSplitNode(centerNodeId, ImGuiDir_Down, 0.25f, nullptr, &centerNodeId);
			auto tileLayerId = ImGui::DockBuilderSplitNode(rightNodeId, ImGuiDir_Down, 0.25f, nullptr, &rightNodeId);

			ImGui::DockBuilderDockWindow(ICON_FA_LIST " Object Details", rightNodeId);
			ImGui::DockBuilderDockWindow(ICON_FA_TH " Tileset", rightNodeId);
			ImGui::DockBuilderDockWindow(ICON_FA_CLIPBOARD_LIST " Tile Details", rightNodeId);
			ImGui::DockBuilderDockWindow(ICON_FA_LAYER_GROUP " Tile Layers", tileLayerId);
			ImGui::DockBuilderDockWindow(ICON_FA_SITEMAP " Scene Hierarchy", leftNodeId);
			ImGui::DockBuilderDockWindow(ICON_FA_IMAGE " Scene", centerNodeId);
			ImGui::DockBuilderDockWindow(ICON_FA_CODE " Script List", centerNodeId);
			ImGui::DockBuilderDockWindow(ICON_FA_ARCHIVE " Package Game", centerNodeId);
			ImGui::DockBuilderDockWindow(ICON_FA_COG " Project Settings", centerNodeId);
			ImGui::DockBuilderDockWindow(ICON_FA_MAP " Tilemap Editor", centerNodeId);
			ImGui::DockBuilderDockWindow(ICON_FA_TERMINAL " Logs", logNodeId);
			ImGui::DockBuilderDockWindow(ICON_FA_FILE_ALT " Assets", logNodeId);
			ImGui::DockBuilderDockWindow(ICON_FA_FOLDER " Content Browser", logNodeId);

			ImGui::DockBuilderFinish(dockSpaceId);
		}
	}

	void Application::RenderDisplays()
	{
		InitDisplays();

		auto& mainRegistry = MAIN_REGISTRY();
		auto& displayHolder = mainRegistry.GetContext<std::shared_ptr<DisplayHolder>>();

		for (const auto& display : displayHolder->displays)
			display->Draw();
	}

	void Application::RegisterEditorMetaFunctions()
	{
		DrawComponentsUtil::RegisterUIComponent<Identification>();
		DrawComponentsUtil::RegisterUIComponent<TransformComponent>();
		DrawComponentsUtil::RegisterUIComponent<SpriteComponent>();
		DrawComponentsUtil::RegisterUIComponent<AnimationComponent>();
		DrawComponentsUtil::RegisterUIComponent<PhysicsComponent>();
		DrawComponentsUtil::RegisterUIComponent<RigidBodyComponent>();
		DrawComponentsUtil::RegisterUIComponent<BoxColliderComponent>();
		DrawComponentsUtil::RegisterUIComponent<CircleColliderComponent>();
		DrawComponentsUtil::RegisterUIComponent<TextComponent>();
	}

	void Application::OnCloseEditor(CloseEditorEvent& close)
	{
		// TODO: Maybe add a check for save?
		m_IsRunning = false;
	}

    Application::Application()
        : m_Window{ nullptr }, m_Event{}, m_IsRunning{ true }
    {}

}
