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

		// Temp texture
		auto texture = assetManager->GetTexture("gem");

		F_TRACE("Texture loaded w:{0} h:{1}", texture.GetWidth(), texture.GetHeight());

		// Create new test entity
		m_Registry = std::make_unique<Feather::Registry>();

		Feather::Entity entity1{ *m_Registry, "TestEntity1", "Test" };

		auto& transform = entity1.AddComponent<Feather::TransformComponent>(Feather::TransformComponent{
						.position = glm::vec2{10.0f, 10.0f},
						.scale = glm::vec2{1.0f, 1.0f},
						.rotation = 0.0f });
		auto& sprite = entity1.AddComponent<Feather::SpriteComponent>(Feather::SpriteComponent{
						.width = 32.0f,
						.height = 32.0f,
						.color = Feather::Color{.r = 0, .g = 255, .b = 0, .a = 255},
						.start_x = 0,
						.start_y = 0 });

		sprite.generate_uvs(texture.GetWidth(), texture.GetHeight());

		std::vector<Feather::Vertex> vertices{};
		Feather::Vertex vTL{}, vTR{}, vBL{}, vBR{};

		vTL.position = glm::vec2{ transform.position.x, transform.position.y + sprite.height };
		vTL.uvs = glm::vec2{ sprite.uvs.u, sprite.uvs.v + sprite.uvs.uv_height };

		vTR.position = glm::vec2{ transform.position.x + sprite.width, transform.position.y + sprite.height };
		vTR.uvs = glm::vec2{ sprite.uvs.u + sprite.uvs.uv_width, sprite.uvs.v + sprite.uvs.uv_height };

		vBL.position = glm::vec2{ transform.position.x, transform.position.y };
		vBL.uvs = glm::vec2{ sprite.uvs.u, sprite.uvs.v };

		vBR.position = glm::vec2{ transform.position.x + sprite.width, transform.position.y };
		vBR.uvs = glm::vec2{ sprite.uvs.u + sprite.uvs.uv_width, sprite.uvs.v };

		vertices.push_back(vTL);
		vertices.push_back(vBL);
		vertices.push_back(vBR);
		vertices.push_back(vTR);

		auto& id = entity1.GetComponent<Feather::Identification>();
		F_INFO("Name: {0}, Group: {1}, ID: {2}", id.name, id.group, id.entity_id);

		GLuint indices[] = {	// 2	1
			0, 1, 2,			//
			2, 3, 0				// 3	0
		};

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

		if (!scriptSystem->LoadMainScript(*lua))
		{
			F_FATAL("Failed to load main lua script");
			return false;
		}

		if (!m_Registry->AddToContext<std::shared_ptr<Feather::ScriptingSystem>>(scriptSystem))
		{
			F_FATAL("Failed to add the script system to the registry context!");
			return false;
		}

		// Camera creation
		auto camera = std::make_shared<Feather::Camera2D>();
		camera->SetScale(5.0f);

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

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		// Bind them
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Feather::Vertex), vertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Feather::Vertex), (void*)offsetof(Feather::Vertex, position));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Feather::Vertex), (void*)offsetof(Feather::Vertex, uvs));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Feather::Vertex), (void*)offsetof(Feather::Vertex, color));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
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
		// Process Events
		while (SDL_PollEvent(&m_Event))
		{
			switch (m_Event.type)
			{
			case SDL_QUIT:
				m_IsRunning = false;
				break;
			case SDL_KEYDOWN:
				if (m_Event.key.keysym.sym = SDLK_ESCAPE)
					m_IsRunning = false;
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
    }

    void Application::Render()
    {
		auto& assetManager = m_Registry->GetContext<std::shared_ptr<Feather::AssetManager>>();
		auto& camera = m_Registry->GetContext<std::shared_ptr<Feather::Camera2D>>();

		auto& shader = assetManager->GetShader("basic");
		auto projection = camera->GetCameraMatrix();

		if (shader.ShaderProgramID() == 0)
		{
			F_FATAL("Shader program has not been created correctly!");
			return;
		}

		glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.Enable();
		glBindVertexArray(VAO);

		shader.SetUniformMat4("u_Projection", projection);

		glActiveTexture(GL_TEXTURE0);
		const auto& texture = assetManager->GetTexture("gem");
		glBindTexture(GL_TEXTURE_2D, texture.GetID());

		auto& scriptSystem = m_Registry->GetContext<std::shared_ptr<Feather::ScriptingSystem>>();
		scriptSystem->Render();

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0);

		SDL_GL_SwapWindow(m_Window->GetWindow().get());

		shader.Disable();
    }

    void Application::CleanUp()
    {
		SDL_Quit();
    }

    Application::Application()
        : m_Window{ nullptr }, m_Registry{ nullptr }, m_Event{}, m_IsRunning{ true }
        , VAO{ 0 }, VBO{ 0 }, IBO{ 0 } // TODO: temporary
    {}

}
