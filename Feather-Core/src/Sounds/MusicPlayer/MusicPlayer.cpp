#include "MusicPlayer.h"

#include "Logger/Logger.h"
#include "Sounds/Essentials/Music.h"

namespace Feather {

	MusicPlayer::MusicPlayer()
	{
		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
		{
			std::string error{ Mix_GetError() };
			F_ERROR("Unable to open SDL Music Mixer: {0}", error);
			return;
		}
		F_TRACE("Channels allocated: '{0}'", Mix_AllocateChannels(16));
	}

	MusicPlayer::~MusicPlayer()
	{
		Mix_HaltMusic();
		Mix_Quit();
		F_INFO("Music Player closed!");
	}

	void MusicPlayer::Play(Music& music, int loops)
	{
		if (!music.GetMusicPtr())
		{
			F_ERROR("Failed to play music '{0}': Mix Music was null!", music.GetName());
			return;
		}

		if (Mix_PlayMusic(music.GetMusicPtr(), loops) != 0)
		{
			std::string error{ Mix_GetError() };
			F_ERROR("Failed to play music '{0}': {1}", music.GetName(), error);
		}
	}

	void MusicPlayer::Pause()
	{
	}

	void MusicPlayer::Resume()
	{
	}

	void MusicPlayer::Stop()
	{
	}

	void MusicPlayer::SetVolume(int volume)
	{
	}

	bool MusicPlayer::IsPlaying()
	{
		return false;
	}

}
