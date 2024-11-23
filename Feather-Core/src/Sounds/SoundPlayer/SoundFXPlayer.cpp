#include "SoundFXPlayer.h"

#include "Logger/Logger.h"
#include "Sounds/Essentials/SoundFX.h"

namespace Feather {

	void SoundFXPlayer::Play(SoundFX& soundFx)
	{
		Play(soundFx, 0, -1);
	}

	void SoundFXPlayer::Play(SoundFX& soundFx, int loops, int channel)
	{
		if (!soundFx.GetSoundFxPtr())
		{
			F_ERROR("Failed to play sound effect '{0}' on channel '{1}': SoundFX ptr is null", soundFx.GetName(), channel);
			return;
		}

		if (Mix_PlayChannel(channel, soundFx.GetSoundFxPtr(), loops) == -1)
		{
			std::string error{ Mix_GetError() };
			F_ERROR("Failed to play sound effect '{0}' on channel '{1}': {2}", soundFx.GetName(), channel, error);
		}
	}

	void SoundFXPlayer::SetVolume(int volume, int channel)
	{
		if (volume < 0 || volume > 100)
		{
			F_ERROR("Failed to set volume. Must be between 0 - 100: Input '{0}'", volume);
			return;
		}

		int volume_changed = static_cast<int>((volume / 100.0f) * 128);
		Mix_Volume(channel, volume_changed);
	}

	void SoundFXPlayer::Stop(int channel)
	{
		if (Mix_HaltChannel(channel) == -1)
		{
			std::string error{ Mix_GetError() };
			F_ERROR("Failed to stop the sound effect for channel '{0}'", channel == -1 ? "all channels" : std::to_string(channel));
		}
	}

	bool SoundFXPlayer::IsPlaying(int channel)
	{
		return Mix_Playing(channel);
	}

}
