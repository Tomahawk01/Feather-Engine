#define SDL_MAIN_HANDLED 1;

#include <Windowing/Window/Window.h>
#include <Renderer/Essentials/ShaderLoader.h>

#include <SDL.h>
#include <glad/glad.h>
#include <SOIL/SOIL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

class Camera2D
{
public:
	Camera2D()
		: Camera2D(640, 480)
	{}

	Camera2D(int width, int height)
		: m_Width{ width }, m_Height{ height }, m_Scale{ 1.0f }, m_Position{ glm::vec2{0} }, m_CameraMatrix{ 1.0f }, m_OrthoProjection{ 1.0f }, m_NeedUpdate{ true }
	{
		m_OrthoProjection = glm::ortho(0.0f, static_cast<float>(m_Width), static_cast<float>(m_Height), 0.0f, 0.0f, 1.0f);
	}

	inline void SetScale(float scale) { m_Scale = scale; m_NeedUpdate = true; }

	inline glm::mat4 GetCameraMatrix() { return m_CameraMatrix; }

	void Update()
	{
		if (!m_NeedUpdate)
			return;
		
		// Translate
		glm::vec3 translate{ -m_Position.x, -m_Position.y, 0.0f };
		m_CameraMatrix = glm::translate(m_OrthoProjection, translate);

		// Scale
		glm::vec3 scale{ m_Scale, m_Scale, 0.0f };
		m_CameraMatrix *= glm::scale(glm::mat4(1.0f), scale);

		m_NeedUpdate = false;
	}

private:
	int m_Width, m_Height;
	float m_Scale;

	glm::vec2 m_Position;
	glm::mat4 m_CameraMatrix, m_OrthoProjection;

	bool m_NeedUpdate;
};

struct UVs
{
	float u, v, width, height;
	UVs()
		: u{ 0.0f }, v{ 0.0f }, width{ 0.0f }, height{ 0.0f }
	{}
};

bool LoadTexture(const std::string& filepath, int& width, int& height, bool blended)
{
	int channels = 0;
	unsigned char* image = SOIL_load_image(filepath.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);

	if (!image)
	{
		std::cout << "Failed to load image '" << filepath << "' - " << SOIL_last_result();
		return false;
	}

	GLint format = GL_RGBA;
	switch (channels)
	{
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (!blended)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);

	SOIL_free_image_data(image);

	return true;
}

int main()
{
	bool running{ true };

	// Init SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::string error = SDL_GetError();
		std::cout << "Failed to initialize SDL: " << error << "\n";
		running = false;
		return -1;
	}

	// Setup OpenGL
	if (SDL_GL_LoadLibrary(NULL) != 0)
	{
		std::string error = SDL_GetError();
		std::cout << "Failed to load OpenGL library: " << error << "\n";
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
		std::cout << "Failed to create the window!" << "\n";
		running = false;
		return -1;
	}

	// Create OpenGL context
	window.SetGLContext(SDL_GL_CreateContext(window.GetWindow().get()));
	if (!window.GetGLContext())
	{
		std::string error = SDL_GetError();
		std::cout << "Failed to create OpenGL context!" << error << "\n";
		running = false;
		return -1;
	}

	SDL_GL_MakeCurrent(window.GetWindow().get(), window.GetGLContext());
	SDL_GL_SetSwapInterval(1);

	// Initialize Glad
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
	{
		std::cout << "Failed to initialize Glad!\n";
		running = false;
		return -1;
	}

	// Enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// TODO: temporary
	// Load Texture
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	int width{ 0 }, height{ 0 };
	if (!LoadTexture("assets/textures/Gem.png", width, height, false))
	{
		std::cout << "Failed to load the texture!\n";
		return -1;
	}

	UVs uvs{};
	auto generateUVs = [&](float startX, float startY, float spriteWidth, float spriteHeight)
	{
		uvs.width = spriteWidth / width;
		uvs.height = spriteHeight / height;
		uvs.u = startX * uvs.width;
		uvs.v = startY * uvs.height;
	};

	generateUVs(0, 0, 32, 32);

	// Flipped tex coords
	float vertices[] = {
		0.0f,  32.0f, 0.0f, uvs.u, (uvs.v + uvs.height),				// 0	last 2 is texture uvs
		0.0f,  0.0f,  0.0f, uvs.u, uvs.v,								// 1
		32.0f, 0.0f,  0.0f, (uvs.u + uvs.width), uvs.v,					// 2
		32.0f, 32.0f, 0.0f, (uvs.u + uvs.width), (uvs.v + uvs.height)	// 3
	};

	GLuint indices[] = {	// 2	1
		0, 1, 2,			//
		2, 3, 0				// 3	0
	};

	// Camera creation
	Camera2D camera{};
	camera.SetScale(5.0f);

	// Shader creation
	auto shader = Feather::ShaderLoader::Create("assets/shaders/basicShader.vert", "assets/shaders/basicShader.frag");
	if (!shader)
	{
		std::cout << "Failed to create shader!\n";
		return -1;
	}

	// Create and generate VAO & VBO & IBO
	GLuint VAO, VBO, IBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Bind them
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * 3 * sizeof(float), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	
	// TODO: temporary end

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
		glBindTexture(GL_TEXTURE_2D, texID);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0);

		SDL_GL_SwapWindow(window.GetWindow().get());

		camera.Update();
		shader->Disable();
	}

	std::cout << "Closing..." << std::endl;
	return 0;
}
