#define SDL_MAIN_HANDLED 1;

#include "Application.h"

int main()
{
#ifndef NDEBUG
	// TODO: hide console
#else
	// TODO: show console
#endif

	auto& app = Feather::Application::GetInstance();
	app.Run();
	
	return 0;
}
