#pragma once

#include <memory>
#include <SDL.h>
#include <SDL_mixer.h>

namespace Feather {

	struct SDL_Destroyer
	{
		void operator()(SDL_Window* window) const;
		void operator()(SDL_GameController* controller) const;
		void operator()(Mix_Chunk* chunk) const;
		void operator()(Mix_Music* music) const;
		void operator()(SDL_Cursor* cursor) const;
	};

}

typedef std::shared_ptr<SDL_GameController> Controller;
static Controller make_shared_controller(SDL_GameController* controller)
{
	return std::shared_ptr<SDL_GameController>(controller, Feather::SDL_Destroyer{});
}

typedef std::shared_ptr<SDL_Cursor> Cursor;
static Cursor make_shared_cursor(SDL_Cursor* cursor);

typedef std::unique_ptr<SDL_Window, Feather::SDL_Destroyer> WindowPtr;

typedef std::unique_ptr<Mix_Chunk, Feather::SDL_Destroyer> SoundFXPtr;
typedef std::unique_ptr<Mix_Music, Feather::SDL_Destroyer> MusicPtr;
