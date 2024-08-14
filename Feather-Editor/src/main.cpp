#define SDL_MAIN_HANDLED 1;

#include "Application.h"

#ifdef _WIN32
#include <Windows.h>
#endif

int main()
{
#ifdef DEBUG
#ifdef _WIN32
	ShowWindow(GetConsoleWindow(), SW_SHOW);
#endif
#else
#ifdef _WIN32
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
#endif

	auto& app = Feather::Application::GetInstance();
	app.Run();
	
	return 0;
}
