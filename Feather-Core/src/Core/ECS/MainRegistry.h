#pragma once
#include "Registry.h"

#include <memory>

#define MAIN_REGISTRY() Feather::MainRegistry::GetInstance()

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

		template<typename TContext>
		TContext AddToContext(TContext context)
		{
			return m_MainRegistry->AddToContext<TContext>(context);
		}

		template<typename TContext>
		TContext& GetContext()
		{
			return m_MainRegistry->GetContext<TContext>();
		}

	private:
		std::unique_ptr<Registry> m_MainRegistry{ nullptr };
		bool m_Initialized{ false };

		MainRegistry() = default;
		~MainRegistry() = default;
		MainRegistry(const MainRegistry&) = delete;
		MainRegistry& operator=(const MainRegistry&) = delete;
	};

}