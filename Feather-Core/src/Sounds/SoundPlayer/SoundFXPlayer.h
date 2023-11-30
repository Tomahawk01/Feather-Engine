#pragma once

namespace Feather {

	class SoundFXPlayer
	{
	public:
		SoundFXPlayer() = default;
		~SoundFXPlayer() = default;

		void Play(class SoundFX& soundFx);
		void Play(class SoundFX& soundFx, int loops, int channel);
		void SetVolume(int channel, int volume);
		void Stop(int channel);
		bool IsPlaying(int channel);
	};

}
