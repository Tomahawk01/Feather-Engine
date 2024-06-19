#pragma once
#include <memory>

#define MAIN_REGISTRY() Feather::MainRegistry::GetInstance();

namespace Feather {

	class AssetManager;
	class MusicPlayer;
	class SoundFXPlayer;

	class MainRegistry
	{
	public:
		static MainRegistry& GetInstance();
		void Initialize();

		AssetManager& GetAssetManager();
		MusicPlayer& GetMusicPlayer();
		SoundFXPlayer& GetSoundPlayer();

	private:
		std::unique_ptr<class Registry> m_MainRegistry{ nullptr };
		bool m_Initialized{ false };

		MainRegistry() = default;
		~MainRegistry() = default;
		MainRegistry(const MainRegistry&) = delete;
		MainRegistry& operator=(const MainRegistry&) = delete;
	};

}