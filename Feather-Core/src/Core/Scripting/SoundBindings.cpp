#include "SoundBindings.h"

#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"
#include "Sounds/MusicPlayer/MusicPlayer.h"
#include "Sounds/SoundPlayer/SoundFXPlayer.h"

namespace Feather {

	void SoundBinder::CreateSoundBind(sol::state& lua, Registry& registry)
	{
		auto& musicPlayer = registry.GetContext<std::shared_ptr<MusicPlayer>>();
		if (!musicPlayer)
		{
			F_ERROR("Failed to bind the music player to Lua: Not in registry!");
			return;
		}

		auto& assetManager = registry.GetContext<std::shared_ptr<AssetManager>>();
		if (!assetManager)
		{
			F_ERROR("Failed to bind the music player to Lua: Asset manager does not exist in registry!");
			return;
		}

		lua.new_usertype<MusicPlayer>(
			"Music",
			sol::no_constructor,
			"play", sol::overload(
				[&](const std::string& musicName, int loops)
				{
					auto music = assetManager->GetMusic(musicName);
					if (!music)
					{
						F_ERROR("Failed to get music '{0}' from the asset manager", musicName);
						return;
					}
					musicPlayer->Play(*music, loops);
				},
				[&](const std::string& musicName)
				{
					auto music = assetManager->GetMusic(musicName);
					if (!music)
					{
						F_ERROR("Failed to get music '{0}' from the asset manager", musicName);
						return;
					}
					musicPlayer->Play(*music, -1);
				}
			),
			"stop", [&]() { musicPlayer->Stop(); },
			"pause", [&]() { musicPlayer->Pause(); },
			"resume", [&]() { musicPlayer->Resume(); },
			"set_volume", [&](int volume) { musicPlayer->SetVolume(volume); },
			"is_playing", [&]() { return musicPlayer->IsPlaying(); }
		);

		auto& soundFxPlayer = registry.GetContext<std::shared_ptr<SoundFXPlayer>>();
		if (!soundFxPlayer)
		{
			F_ERROR("Failed to bind the sound effects player to Lua: Not in registry!");
			return;
		}

		lua.new_usertype<SoundFXPlayer>(
			"Sound",
			sol::no_constructor,
			"play", sol::overload(
				[&](const std::string& soundName)
				{
					auto soundFx = assetManager->GetSoundFx(soundName);
					if (!soundFx)
					{
						F_ERROR("Failed to get sound effect '{0}' from the asset manager", soundName);
						return;
					}
					soundFxPlayer->Play(*soundFx);
				},
				[&](const std::string& soundName, int loops, int channel)
				{
					auto soundFx = assetManager->GetSoundFx(soundName);
					if (!soundFx)
					{
						F_ERROR("Failed to get sound effect '{0}' from the asset manager", soundName);
						return;
					}
					soundFxPlayer->Play(*soundFx, loops, channel);
				}
			),
			"stop", [&](int channel) { soundFxPlayer->Stop(channel); },
			"set_volume", [&](int channel, int volume) { soundFxPlayer->SetVolume(channel, volume); },
			"is_playing", [&](int channel) { return soundFxPlayer->IsPlaying(channel); }
		);
	}

}
