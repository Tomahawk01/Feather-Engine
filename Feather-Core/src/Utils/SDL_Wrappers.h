#pragma once

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

// Useful aliases
using Controller = std::shared_ptr<SDL_GameController>;
using Cursor = std::shared_ptr<SDL_Cursor>;
using WindowPtr = std::unique_ptr<SDL_Window, Feather::SDL_Destroyer>;
using SoundFXPtr = std::unique_ptr<Mix_Chunk, Feather::SDL_Destroyer>;
using MusicPtr = std::unique_ptr<Mix_Music, Feather::SDL_Destroyer>;

template <typename RPtr, typename TSDLType>
inline RPtr MakeSharedFromSDLType(TSDLType* SDLType)
{
	return std::shared_ptr<TSDLType>(SDLType, Feather::SDL_Destroyer{});
}
