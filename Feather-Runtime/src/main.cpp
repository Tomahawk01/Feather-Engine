#define SDL_MAIN_HANDLED 1;

#include "Runtime.h"

#ifdef _WIN32
#include <Windows.h>
#endif

int main()
{
#ifdef _WIN32
#ifdef DEBUG
	ShowWindow(GetConsoleWindow(), 1);
#else
	ShowWindow(GetConsoleWindow(), 0);
#endif
#endif

	Feather::RuntimeApp app{};
	app.Run();

	return 0;
}
