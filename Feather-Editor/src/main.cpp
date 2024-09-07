#define SDL_MAIN_HANDLED 1;

#include "Application.h"

#ifdef _WIN32
#include <Windows.h>
#endif

int main()
{
#ifdef DEBUG
#ifdef _WIN32
	ShowWindow(GetConsoleWindow(), 1);
#endif
#else
#ifdef _WIN32
	ShowWindow(GetConsoleWindow(), 0);
#endif
#endif

	auto& app = Feather::Application::GetInstance();
	app.Run();
	
	return 0;
}
