#define SDL_MAIN_HANDLED 1;
#define NIMINMAX // include in premake?

#include <Windowing/Window/Window.h>
#include <Renderer/Essentials/ShaderLoader.h>
#include <Renderer/Essentials/TextureLoader.h>
#include <Renderer/Essentials/Vertex.h>
#include <Renderer/Core/Camera2D.h>
#include <Logger/Logger.h>

#include <entt.hpp>

struct UVs
{
	float u{ 0.0f }, v{ 0.0f }, uv_width{ 0.0f }, uv_height{ 0.0f };
};

struct TransformComponent
{
	glm::vec2 position{ glm::vec2{0.0f} }, scale{ glm::vec2 {1.0f} };
	float rotation{ 0.0f };
};

struct SpriteComponent
{
	float width{ 0.0f }, height{ 0.0f };
	UVs uvs{ .u = 0.0f, .v = 0.0f, .uv_width = 0.0f, .uv_height = 0.0f };

	Feather::Color color{ .r = 255, .g = 255, .b = 255, .a = 255 };
	int start_x{ 0 }, start_y{ 0 };

	void generate_uvs(int textureWidth, int textureHeight)
	{
		uvs.uv_width = width / textureWidth;
		uvs.uv_height = height / textureHeight;

		uvs.u = start_x * uvs.uv_width;
		uvs.v = start_y * uvs.uv_height;
	}
};

int main()
{
	Feather::Log::Init();

	bool running{ true };

	// Init SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::string error = SDL_GetError();
		F_ERROR("Failed to initialize SDL! {0}", error);
		running = false;
		return -1;
	}

	// Setup OpenGL
	if (SDL_GL_LoadLibrary(NULL) != 0)
	{
		std::string error = SDL_GetError();
		F_ERROR("Failed to load OpenGL library! {0}", error);
		running = false;
		return -1;
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
	Feather::Window window("Test Window", 640, 480, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, true, SDL_WINDOW_OPENGL);

	if (!window.GetWindow())
	{
		F_ERROR("Failed to create the window!");
		running = false;
		return -1;
	}

	// Create OpenGL context
	window.SetGLContext(SDL_GL_CreateContext(window.GetWindow().get()));
	if (!window.GetGLContext())
	{
		std::string error = SDL_GetError();
		F_ERROR("Failed to create OpenGL context! {0}", error);
		running = false;
		return -1;
	}

	SDL_GL_MakeCurrent(window.GetWindow().get(), window.GetGLContext());
	SDL_GL_SetSwapInterval(1);

	// Initialize Glad
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
	{
		F_ERROR("Failed to initialize Glad!");
		running = false;
		return -1;
	}

	// Enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create registry
	auto pRegistry = std::make_unique<entt::registry>();
	if (!pRegistry)
	{
		F_ERROR("Failed to create entt registry!");
		return -1;
	}

	// Temp texture
	auto texture = Feather::TextureLoader::Create(Feather::Texture::TextureType::PIXEL, "assets/textures/Gem.png");
	if (!texture)
	{
		F_ERROR("Failed to create texture!");
		return -1;
	}

	// Temp UVs
	UVs uVs{};
	F_TRACE("Texture loaded w:{0} h:{1}", texture->GetWidth(), texture->GetHeight());

	// Create new test entity
	auto ent1 = pRegistry->create();
	auto& transform = pRegistry->emplace<TransformComponent>(ent1, TransformComponent{
					.position = glm::vec2{10.0f, 10.0f},
					.scale = glm::vec2{1.0f, 1.0f},
					.rotation = 0.0f });
	auto& sprite = pRegistry->emplace<SpriteComponent>(ent1, SpriteComponent{
					.width = 32.0f,
					.height = 32.0f,
					.color = Feather::Color{.r = 0, .g = 255, .b = 0, .a = 255},
					.start_x = 0,
					.start_y = 0 });

	sprite.generate_uvs(texture->GetWidth(), texture->GetHeight());

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

	GLuint indices[] = {	// 2	1
		0, 1, 2,			//
		2, 3, 0				// 3	0
	};

	// Camera creation
	Feather::Camera2D camera{};
	camera.SetScale(5.0f);

	// Shader creation
	auto shader = Feather::ShaderLoader::Create("assets/shaders/basicShader.vert", "assets/shaders/basicShader.frag");
	if (!shader)
	{
		F_ERROR("Failed to create shader!");
		return -1;
	}

	// Create and generate VAO & VBO & IBO
	GLuint VAO, VBO, IBO;

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

	SDL_Event event{};

	// Window loop
	while (running)
	{
		// Process Events
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym = SDLK_ESCAPE)
					running = false;
				break;
			default:
				break;
			}
		}

		glViewport(0, 0, window.GetWidth(), window.GetHeight());

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader->Enable();
		glBindVertexArray(VAO);

		auto projection = camera.GetCameraMatrix();
		shader->SetUniformMat4("u_Projection", projection);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->GetID());

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0);

		SDL_GL_SwapWindow(window.GetWindow().get());

		camera.Update();
		shader->Disable();
	}

	F_INFO("Closing...");
	return 0;
}
