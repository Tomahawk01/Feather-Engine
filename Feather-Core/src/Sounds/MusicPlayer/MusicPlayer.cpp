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

		if ((Mix_Init(MIX_INIT_MP3) & MIX_INIT_MP3) == 0)
		{
			F_ERROR("MP3 support is not available: {}", Mix_GetError());
		}
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
		Mix_PauseMusic();
	}

	void MusicPlayer::Resume()
	{
		Mix_ResumeMusic();
	}

	void MusicPlayer::Stop()
	{
		Mix_HaltMusic();
	}

	void MusicPlayer::SetVolume(int volume)
	{
		if (volume < 0 || volume > 100)
		{
			F_ERROR("Failed to set volume. Must be between 0 - 100: Input '{0}'", volume);
			return;
		}

		int volume_changed = static_cast<int>((volume / 100.0f) * 128);
		Mix_VolumeMusic(volume_changed);
	}

	bool MusicPlayer::IsPlaying()
	{
		return Mix_PlayingMusic();
	}

}
