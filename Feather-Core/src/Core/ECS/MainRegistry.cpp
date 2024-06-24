#include "MainRegistry.h"
#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"
#include "Sounds/MusicPlayer/MusicPlayer.h"
#include "Sounds/SoundPlayer/SoundFXPlayer.h"

namespace Feather {

    MainRegistry& MainRegistry::GetInstance()
    {
        static MainRegistry instance{};
        return instance;
    }

    void MainRegistry::Initialize()
    {
        m_MainRegistry = std::make_unique<Registry>();
        F_ASSERT(m_MainRegistry && "Failed to initialize main registry");

        auto pAssetManager = std::make_shared<AssetManager>();
        m_MainRegistry->AddToContext<std::shared_ptr<AssetManager>>(std::move(pAssetManager));

        auto pMusicPlayer = std::make_shared<MusicPlayer>();
        m_MainRegistry->AddToContext<std::shared_ptr<MusicPlayer>>(std::move(pMusicPlayer));

        auto pSoundPlayer = std::make_shared<SoundFXPlayer>();
        m_MainRegistry->AddToContext<std::shared_ptr<SoundFXPlayer>>(std::move(pSoundPlayer));

        m_Initialized = true;
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

}
