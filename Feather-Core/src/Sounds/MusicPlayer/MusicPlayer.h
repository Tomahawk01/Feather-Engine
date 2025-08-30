#pragma once

namespace Feather {
	
	class MusicPlayer
	{
	public:
		MusicPlayer();
		~MusicPlayer();

		void Play(class Music& music, int loops = 0);
		void Pause();
		void Resume();
		void Stop();
		void SetVolume(int volume);
		bool IsPlaying();
		bool IsPaused();
	};

}
