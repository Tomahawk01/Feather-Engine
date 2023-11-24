#define SDL_MAIN_HANDLED 1;

#include <Windowing/Window/Window.h>
#include <SDL.h>
#include <glad/glad.h>

#include <iostream>

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
	Feather::Window window("Test Window", 640, 640, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, true, SDL_WINDOW_OPENGL);

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

	// TODO: temporary
	float vertices[] = {
		-0.5f,  0.5f, 0.0f,	// 0
		 0.5f,  0.5f, 0.0f,	// 1
		 0.5f, -0.5f, 0.0f,	// 2
		-0.5f, -0.5f, 0.0f,	// 3
	};

	GLuint indices[] = {	// 2	1
		0, 1, 2,			//
		2, 3, 0				// 3	0
	};

	// Create VERTEX Shader
	const char* vertexSource =
		"#version 450 core\n"
		"layout (location = 0) in vec3 in_Position;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(in_Position, 1.0);\n"
		"}";

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vertexSource, nullptr);
	glCompileShader(vertexShader);

	int status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog);
		std::cout << "Failed to compile vertex shader!\n" << infoLog << "\n";
		return -1;
	}

	// Create FRAGMENT Shader
	const char* fragmentSource =
		"#version 450 core\n"
		"layout (location = 0) out vec4 out_Color;\n"
		"void main()\n"
		"{\n"
		"	out_Color = vec4(0.2, 0.2, 0.8, 1.0);\n"
		"}";

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog);
		std::cout << "Failed to compile fragment shader!\n" << infoLog << "\n";
		return -1;
	}

	// Create shader program
	GLuint shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	// Link the program
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
	if (!status)
	{
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);
		std::cout << "Failed to link shader program!\n" << infoLog << "\n";
		return -1;
	}

	// Enable shader program
	glUseProgram(shaderProgram);

	// Delete shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

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

	glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

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

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0);

		SDL_GL_SwapWindow(window.GetWindow().get());
	}

	std::cout << "Closing..." << std::endl;
	return 0;
}
