#include "SoundBindings.h"

#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/MainRegistry.h"
#include "Sounds/MusicPlayer/MusicPlayer.h"
#include "Sounds/SoundPlayer/SoundFXPlayer.h"

namespace Feather {

	void SoundBinder::CreateSoundBind(sol::state& lua)
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& musicPlayer = mainRegistry.GetMusicPlayer();
		auto& assetManager = mainRegistry.GetAssetManager();

		lua.new_usertype<MusicPlayer>(
			"Music",
			sol::no_constructor,
			"play", sol::overload(
				[&](const std::string& musicName, int loops)
				{
					auto pMusic = assetManager.GetMusic(musicName);
					if (!pMusic)
					{
						F_ERROR("Failed to get music '{0}' from the asset manager", musicName);
						return;
					}
					musicPlayer.Play(*pMusic, loops);
				},
				[&](const std::string& musicName)
				{
					auto pMusic = assetManager.GetMusic(musicName);
					if (!pMusic)
					{
						F_ERROR("Failed to get music '{0}' from the asset manager", musicName);
						return;
					}
					musicPlayer.Play(*pMusic, -1);
				}
			),
			"stop", [&]() { musicPlayer.Stop(); },
			"pause", [&]() { musicPlayer.Pause(); },
			"resume", [&]() { musicPlayer.Resume(); },
			"setVolume", [&](int volume) { musicPlayer.SetVolume(volume); },
			"isPlaying", [&]() { return musicPlayer.IsPlaying(); },
			"isPaused", [&]() { return musicPlayer.IsPaused(); }
		);

		auto& soundFxPlayer = mainRegistry.GetSoundPlayer();

		lua.new_usertype<SoundFXPlayer>(
			"Sound",
			sol::no_constructor,
			"play", sol::overload(
				[&](const std::string& soundName)
				{
					auto soundFx = assetManager.GetSoundFx(soundName);
					if (!soundFx)
					{
						F_ERROR("Failed to get sound effect '{0}' from the asset manager", soundName);
						return;
					}
					soundFxPlayer.Play(*soundFx);
				},
				[&](const std::string& soundName, int loops, int channel)
				{
					auto soundFx = assetManager.GetSoundFx(soundName);
					if (!soundFx)
					{
						F_ERROR("Failed to get sound effect '{0}' from the asset manager", soundName);
						return;
					}
					soundFxPlayer.Play(*soundFx, loops, channel);
				}
			),
			"stop", [&](int channel) { soundFxPlayer.Stop(channel); },
			"setVolume", [&](int channel, int volume) { soundFxPlayer.SetVolume(channel, volume); },
			"isPlaying", [&](int channel) { return soundFxPlayer.IsPlaying(channel); }
		);
	}

}
