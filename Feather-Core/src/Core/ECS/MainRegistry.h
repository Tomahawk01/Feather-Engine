#pragma once
#include "Registry.h"

#include <memory>

#define MAIN_REGISTRY() Feather::MainRegistry::GetInstance()
#define ASSET_MANAGER() MAIN_REGISTRY().GetAssetManager()

namespace Feather {

	class AssetManager;
	class MusicPlayer;
	class SoundFXPlayer;

	class RenderSystem;
	class RenderUISystem;
	class RenderShapeSystem;
	class AnimationSystem;
	class PhysicsSystem;

	class MainRegistry
	{
	public:
		static MainRegistry& GetInstance();
		bool Initialize();

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

		RenderSystem& GetRenderSystem();
		RenderUISystem& GetRenderUISystem();
		RenderShapeSystem& GetRenderShapeSystem();
		AnimationSystem& GetAnimationSystem();
		PhysicsSystem& GetPhysicsSystem();

	private:
		std::unique_ptr<Registry> m_MainRegistry{ nullptr };
		bool m_Initialized{ false };

		MainRegistry() = default;
		~MainRegistry() = default;
		MainRegistry(const MainRegistry&) = delete;
		MainRegistry& operator=(const MainRegistry&) = delete;

		bool RegisterMainSystems();
	};

}