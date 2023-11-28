#include "SDL_Wrappers.h"
#include "Logger/Logger.h"

void Feather::SDL_Destroyer::operator()(SDL_Window* window) const
{
	SDL_DestroyWindow(window);
	F_INFO("SDL Window Destroyed!");
}

void Feather::SDL_Destroyer::operator()(SDL_GameController* controller) const
{
	SDL_GameControllerClose(controller);
	controller = nullptr;
	F_INFO("Closed SDL Game Controller!");
}

void Feather::SDL_Destroyer::operator()(SDL_Cursor* cursor) const
{
}

Cursor make_shared_cursor(SDL_Cursor* cursor)
{
	return Cursor();
}
