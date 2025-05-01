#include "MainRegistry.h"
#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"
#include "Sounds/MusicPlayer/MusicPlayer.h"
#include "Sounds/SoundPlayer/SoundFXPlayer.h"

#include "Core/Systems/ScriptingSystem.h"
#include "Core/Systems/RenderSystem.h"
#include "Core/Systems/RenderUISystem.h"
#include "Core/Systems/RenderShapeSystem.h"
#include "Core/Systems/AnimationSystem.h"
#include "Core/Systems/PhysicsSystem.h"
#ifdef IN_FEATHER_EDITOR
#include "Core/Systems/RenderPickingSystem.h"
#endif
#include "Core/Events/EventDispatcher.h"
#include "Renderer/Core/Renderer.h"
#include "Utils/HelperUtilities.h"

namespace Feather {

    MainRegistry& MainRegistry::GetInstance()
    {
        static MainRegistry instance{};
        return instance;
    }

    bool MainRegistry::Initialize()
    {
        m_MainRegistry = std::make_unique<Registry>();
        F_ASSERT(m_MainRegistry && "Failed to initialize main registry");

        auto pAssetManager = std::make_shared<AssetManager>();
        m_MainRegistry->AddToContext<std::shared_ptr<AssetManager>>(std::move(pAssetManager));

        auto pMusicPlayer = std::make_shared<MusicPlayer>();
        m_MainRegistry->AddToContext<std::shared_ptr<MusicPlayer>>(std::move(pMusicPlayer));

        auto pSoundPlayer = std::make_shared<SoundFXPlayer>();
        m_MainRegistry->AddToContext<std::shared_ptr<SoundFXPlayer>>(std::move(pSoundPlayer));

        auto renderer = std::make_shared<Renderer>();

        // Enable alpha blending
        renderer->SetCapability(Renderer::GLCapability::BLEND, true);
        renderer->SetCapability(Renderer::GLCapability::DEPTH_TEST, true);
        renderer->SetBlendCapability(
            Renderer::BlendingFactors::SRC_ALPHA,
            Renderer::BlendingFactors::ONE_MINUS_SRC_ALPHA
        );
        renderer->SetLineWidth(4.0f);

        if (!AddToContext<std::shared_ptr<Renderer>>(renderer))
        {
            F_FATAL("Failed to add the renderer to the registry context!");
            return false;
        }

        m_Initialized = RegisterMainSystems();

        return m_Initialized;
    }

    bool MainRegistry::RegisterMainSystems()
    {
        auto renderSystem = std::make_shared<RenderSystem>();
        if (!renderSystem)
        {
            F_FATAL("Failed to create the render system!");
            return false;
        }
        if (!AddToContext<std::shared_ptr<RenderSystem>>(renderSystem))
        {
            F_FATAL("Failed to add the render system to the registry context!");
            return false;
        }

        auto renderUISystem = std::make_shared<RenderUISystem>();
        if (!renderUISystem)
        {
            F_FATAL("Failed to create the render UI system!");
            return false;
        }
        if (!AddToContext<std::shared_ptr<RenderUISystem>>(renderUISystem))
        {
            F_FATAL("Failed to add the render UI system to the registry context!");
            return false;
        }

        auto renderShapeSystem = std::make_shared<RenderShapeSystem>();
        if (!renderShapeSystem)
        {
            F_FATAL("Failed to create the render Shape system!");
            return false;
        }
        if (!AddToContext<std::shared_ptr<RenderShapeSystem>>(renderShapeSystem))
        {
            F_FATAL("Failed to add the render Shape system to the registry context!");
            return false;
        }

        AddToContext<std::shared_ptr<PhysicsSystem>>(std::make_shared<PhysicsSystem>());
        AddToContext<std::shared_ptr<AnimationSystem>>(std::make_shared<AnimationSystem>());
        AddToContext<std::shared_ptr<EventDispatcher>>(std::make_shared<EventDispatcher>());
#ifdef IN_FEATHER_EDITOR
        AddToContext<std::shared_ptr<RenderPickingSystem>>(std::make_shared<RenderPickingSystem>());
        F_TRACE("Added Render Picking System to main registry");
#endif

        return true;
    }

    AssetManager& MainRegistry::GetAssetManager()
    {
        F_ASSERT(m_Initialized && "Main Registry must be initialized before use");
        return *m_MainRegistry->GetContext<std::shared_ptr<AssetManager>>();
    }

    MusicPlayer& MainRegistry::GetMusicPlayer()
    {
        F_ASSERT(m_Initialized && "Main Registry must be initialized before use");
        return *m_MainRegistry->GetContext<std::shared_ptr<MusicPlayer>>();
    }

    SoundFXPlayer& MainRegistry::GetSoundPlayer()
    {
        F_ASSERT(m_Initialized && "Main Registry must be initialized before use");
        return *m_MainRegistry->GetContext<std::shared_ptr<SoundFXPlayer>>();
    }

    RenderSystem& MainRegistry::GetRenderSystem()
    {
        F_ASSERT(m_Initialized && "Main Registry must be initialized before use");
        return *m_MainRegistry->GetContext<std::shared_ptr<RenderSystem>>();
    }

    RenderUISystem& MainRegistry::GetRenderUISystem()
    {
        F_ASSERT(m_Initialized && "Main Registry must be initialized before use");
        return *m_MainRegistry->GetContext<std::shared_ptr<RenderUISystem>>();
    }

    RenderShapeSystem& MainRegistry::GetRenderShapeSystem()
    {
        F_ASSERT(m_Initialized && "Main Registry must be initialized before use");
        return *m_MainRegistry->GetContext<std::shared_ptr<RenderShapeSystem>>();
    }

    AnimationSystem& MainRegistry::GetAnimationSystem()
    {
        F_ASSERT(m_Initialized && "Main Registry must be initialized before use");
        return *m_MainRegistry->GetContext<std::shared_ptr<AnimationSystem>>();
    }

    PhysicsSystem& MainRegistry::GetPhysicsSystem()
    {
        F_ASSERT(m_Initialized && "Main Registry must be initialized before use");
        return *m_MainRegistry->GetContext<std::shared_ptr<PhysicsSystem>>();
    }

    EventDispatcher& MainRegistry::GetEventDispatcher()
    {
        F_ASSERT(m_Initialized && "Main Registry must be initialized before use");
        return *m_MainRegistry->GetContext<std::shared_ptr<EventDispatcher>>();
    }

}
