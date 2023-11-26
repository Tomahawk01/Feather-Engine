#define SDL_MAIN_HANDLED 1;
#define NOMINMAX

#include "Application.h"

int main()
{
	auto& app = Feather::Application::GetInstance();
	app.Run();
	
	return 0;
}
