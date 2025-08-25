#pragma once

#include "Registry.h"

#define MAIN_REGISTRY() Feather::MainRegistry::GetInstance()
#define ASSET_MANAGER() MAIN_REGISTRY().GetAssetManager()
#define EVENT_DISPATCHER() MAIN_REGISTRY().GetEventDispatcher()
#define ADD_EVENT_HANDLER(Event, Func, Handler) EVENT_DISPATCHER().AddHandler<Event, Func>(Handler);

namespace Feather {

	class AssetManager;
	class MusicPlayer;
	class SoundFXPlayer;

	class Renderer;
	class RenderSystem;
	class RenderUISystem;
	class RenderShapeSystem;
	class AnimationSystem;
	class PhysicsSystem;

	class EventDispatcher;

	class MainRegistry
	{
	public:
		static MainRegistry& GetInstance();
		bool Initialize(bool enableFilewatcher = false);

		AssetManager& GetAssetManager();
		MusicPlayer& GetMusicPlayer();
		SoundFXPlayer& GetSoundPlayer();
		Renderer& GetRenderer();

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
		Registry* GetRegistry();

		EventDispatcher& GetEventDispatcher();

	private:
		MainRegistry() = default;
		~MainRegistry() = default;
		MainRegistry(const MainRegistry&) = delete;
		MainRegistry& operator=(const MainRegistry&) = delete;

		bool RegisterMainSystems();

	private:
		std::unique_ptr<Registry> m_MainRegistry{ nullptr };
		bool m_Initialized{ false };
	};

}