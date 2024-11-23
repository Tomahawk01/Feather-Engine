#pragma once

namespace Feather {

	class SoundFXPlayer
	{
	public:
		SoundFXPlayer() = default;
		~SoundFXPlayer() = default;

		void Play(class SoundFX& soundFx);
		void Play(class SoundFX& soundFx, int loops, int channel = -1);
		void SetVolume(int volume, int channel = -1);
		void Stop(int channel);
		bool IsPlaying(int channel);
	};

}
