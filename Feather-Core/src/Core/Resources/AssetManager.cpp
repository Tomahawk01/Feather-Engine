#include "AssetManager.h"

#include "Logger/Logger.h"
#include "Renderer/Essentials/TextureLoader.h"
#include "Renderer/Essentials/ShaderLoader.h"
#include "Renderer/Essentials/FontLoader.h"
#include "Renderer/Essentials/Shader.h"
#include "Renderer/Essentials/Texture.h"
#include "Renderer/Essentials/Font.h"
#include "Sounds/Essentials/Music.h"
#include "Sounds/Essentials/SoundFX.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/Registry.h"
#include "Core/CoreUtils/Prefab.h"

#include "Core/Resources/fonts/default_fonts.h"

#include "Utils/FeatherUtilities.h"
#include "Utils/SDL_Wrappers.h"

#include <SDL_image.h>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

namespace Feather {

    AssetManager::AssetManager(bool enableFilewatcher)
        : m_FileWatcherRunning{ enableFilewatcher }
    {
        if (enableFilewatcher)
        {
            m_WatchThread = std::jthread(&AssetManager::FileWatcher, this);
        }

#ifdef IN_FEATHER_EDITOR
        IMG_Init(IMG_INIT_PNG);
        m_mapCursors.emplace("default", MakeSharedFromSDLType<Cursor>(SDL_GetDefaultCursor()));
#endif
    }

    AssetManager::~AssetManager()
    {
        m_FileWatcherRunning = false;
        if (m_WatchThread.joinable())
        {
            m_WatchThread.join();
        }
    }

    bool AssetManager::CreateDefaultFonts()
    {
        if (!AddFontFromMemory("pixelFont-32", g_PixelFont))
        {
            F_ERROR("Failed to create default pixel font");
            return false;
        }

        if (!AddFontFromMemory("roboto-bold-32", g_PixelFont))
        {
            F_ERROR("Failed to create roboto font");
            return false;
        }

        // TODO: Add more default fonts

        return true;
    }

    bool AssetManager::AddTexture(const std::string& textureName, const std::string& texturePath, bool pixelArt, bool isTileset)
    {
        if (m_mapTextures.find(textureName) != m_mapTextures.end())
        {
            F_ERROR("Failed to add texture '{0}': Already exists!", textureName);
            return false;
        }

        auto texture = TextureLoader::Create(pixelArt ? Texture::TextureType::PIXEL : Texture::TextureType::BLENDED,
                                             texturePath,
                                             isTileset);
        if (!texture)
        {
            F_ERROR("Failed to load texture '{0}' at path '{1}'", textureName, texturePath);
            return false;
        }

        auto [itr, isSuccess] = m_mapTextures.emplace(textureName, std::move(texture));

        if (m_FileWatcherRunning && isSuccess)
        {
            std::lock_guard lock{ m_AssetMutex };

            fs::path path{ texturePath };
            auto lastWrite = fs::last_write_time(path);
            if (CheckContainsValue(m_FilewatchParams, [&](const auto& params) { return params.filepath == texturePath; }))
            {
                m_FilewatchParams.emplace_back(
                    AssetWatchParams{
                        .assetName = textureName,
                        .filepath = texturePath,
                        .lastWrite = lastWrite,
                        .type = AssetType::TEXTURE });
            }
        }

        return isSuccess;
    }

    bool AssetManager::AddTextureFromMemory(const std::string& textureName, const unsigned char* imageData, size_t length, bool pixelArt, bool isTileset)
    {
        if (m_mapTextures.contains(textureName))
        {
            F_ERROR("AssetManager: Texture '{}': Already exists!", textureName);
            return false;
        }

        auto texture = TextureLoader::CreateFromMemory(imageData, length, !pixelArt, isTileset);
        // Load texture
        if (!texture)
        {
            F_ERROR("Unable to load texture '{}' from memory", textureName);
            return false;
        }

        // Insert texture into the map
        auto [itr, isSuccess] = m_mapTextures.emplace(textureName, std::move(texture));

        return isSuccess;
    }

    std::shared_ptr<Texture> AssetManager::GetTexture(const std::string& textureName)
    {
        auto texIter = m_mapTextures.find(textureName);
        if (texIter == m_mapTextures.end())
        {
            F_ERROR("Failed to get texture '{0}': Does not exist!", textureName);
            return nullptr;
        }

        return texIter->second;
    }

    std::vector<std::string> AssetManager::GetTilesetNames() const
    {
        return GetKeys(m_mapTextures, [](const auto& pair) { return pair.second->IsTileset(); });
    }

    bool AssetManager::AddFont(const std::string& fontName, const std::string& fontPath, float fontSize)
    {
        if (m_mapFonts.contains(fontName))
        {
            F_ERROR("Failed to add font '{0}': Already Exists!", fontName);
            return false;
        }

        auto pFont = FontLoader::Create(fontPath, fontSize);

        if (!pFont)
        {
            F_ERROR("Failed to add font '{0}' at path '{1}' to the asset manager!", fontName, fontPath);
            return false;
        }

        auto [itr, isSuccess] = m_mapFonts.emplace(fontName, std::move(pFont));

        if (m_FileWatcherRunning && isSuccess)
        {
            std::lock_guard lock{ m_AssetMutex };

            fs::path path{ fontPath };
            auto lastWrite = fs::last_write_time(path);
            if (CheckContainsValue(m_FilewatchParams, [&](const auto& params) { return params.filepath == fontPath; }))
            {
                m_FilewatchParams.emplace_back(
                    AssetWatchParams{ .assetName = fontName,
                                      .filepath = fontPath,
                                      .lastWrite = lastWrite,
                                      .type = AssetType::FONT });
            }
        }

        return isSuccess;
    }

    bool AssetManager::AddFontFromMemory(const std::string& fontName, unsigned char* fontData, float fontSize)
    {
        if (m_mapFonts.contains(fontName))
        {
            F_ERROR("Failed to add font '{0}': Already Exists!", fontName);
            return false;
        }

        auto pFont = FontLoader::CreateFromMemory(fontData, fontSize);

        if (!pFont)
        {
            F_ERROR("Failed to add font '{0}' from memory to the asset manager!", fontName);
            return false;
        }

        auto [itr, isSuccess] = m_mapFonts.emplace(fontName, std::move(pFont));

        return isSuccess;
    }

    std::shared_ptr<Font> AssetManager::GetFont(const std::string& fontName)
    {
        auto fontItr = m_mapFonts.find(fontName);
        if (fontItr == m_mapFonts.end())
        {
            F_ERROR("Failed to get font '{0}': Does not exist!", fontName);
            return nullptr;
        }

        return fontItr->second;
    }

    bool AssetManager::AddShader(const std::string& shaderName, const std::string& vertexPath, const std::string& fragmentPath)
    {
        if (m_mapShaders.contains(shaderName))
        {
            F_ERROR("Failed to add shader '{0}': Already exists!", shaderName);
            return false;
        }

        auto shader = ShaderLoader::Create(vertexPath, fragmentPath);
        if (!shader)
        {
            F_ERROR("Failed to load shader '{0}' at vert path '{1}' and frag path '{2}'", shaderName, vertexPath, fragmentPath);
            return false;
        }

        auto [itr, isSuccess] = m_mapShaders.emplace(shaderName, std::move(shader));

        if (m_FileWatcherRunning && isSuccess)
        {
            std::lock_guard lock{ m_AssetMutex };

            fs::path pathVert{ vertexPath };
            auto lastWriteVert = fs::last_write_time(pathVert);
            if (CheckContainsValue(m_FilewatchParams, [&](const auto& params) { return params.filepath == vertexPath; }))
            {
                m_FilewatchParams.emplace_back(
                    AssetWatchParams{ .assetName = shaderName + "_vert",
                                      .filepath = vertexPath,
                                      .lastWrite = lastWriteVert,
                                      .type = AssetType::SHADER });
            }

            fs::path pathFrag{ fragmentPath };
            auto lastWriteFrag = fs::last_write_time(pathFrag);
            if (CheckContainsValue(m_FilewatchParams, [&](const auto& params) { return params.filepath == fragmentPath; }))
            {
                m_FilewatchParams.emplace_back(
                    AssetWatchParams{ .assetName = shaderName + "_frag",
                                      .filepath = fragmentPath,
                                      .lastWrite = lastWriteFrag,
                                      .type = AssetType::SHADER });
            }
        }

        return isSuccess;
    }

    bool AssetManager::AddShaderFromMemory(const std::string& shaderName, const char* vertexShader, const char* fragmentShader)
    {
        if (m_mapShaders.contains(shaderName))
        {
            F_ERROR("Failed to add shader - '{0}' -- Already exists!", shaderName);
            return false;
        }

        auto shader = ShaderLoader::CreateFromMemory(vertexShader, fragmentShader);
        auto [itr, isSuccess] = m_mapShaders.insert(std::make_pair(shaderName, std::move(shader)));

        return isSuccess;
    }

    std::shared_ptr<Shader> AssetManager::GetShader(const std::string& shaderName)
    {
        auto shaderIter = m_mapShaders.find(shaderName);
        if (shaderIter == m_mapShaders.end())
        {
            F_ERROR("Failed to get shader '{0}': Does not exist!", shaderName);
            return nullptr;
        }

        return shaderIter->second;
    }

    bool AssetManager::AddMusic(const std::string& musicName, const std::string& filepath)
    {
        if (m_mapMusic.contains(musicName))
        {
            F_ERROR("Failed to add music '{0}': Already exist!", musicName);
            return false;
        }

        Mix_Music* music = Mix_LoadMUS(filepath.c_str());

        if (!music)
        {
            std::string error{ Mix_GetError() };
            F_ERROR("Failed to load '{0}' at path '{1}': Mixer Error - {2}", musicName, filepath, error);
            return false;
        }

        SoundParams params{ .name = musicName, .filename = filepath, .duration = Mix_MusicDuration(music) };

        auto musicPtr = std::make_shared<Music>(params, MusicPtr{ music });
        if (!musicPtr)
        {
            F_ERROR("Failed to create music ptr for '{0}'", musicName);
            return false;
        }

        auto [itr, isSuccess] = m_mapMusic.emplace(musicName, std::move(musicPtr));

        if (m_FileWatcherRunning && isSuccess)
        {
            std::lock_guard lock{ m_AssetMutex };

            fs::path path{ filepath };
            auto lastWrite = fs::last_write_time(path);
            if (CheckContainsValue(m_FilewatchParams, [&](const auto& params) { return params.filepath == filepath; }))
            {
                m_FilewatchParams.emplace_back(
                    AssetWatchParams{ .assetName = musicName,
                                      .filepath = filepath,
                                      .lastWrite = lastWrite,
                                      .type = AssetType::MUSIC });
            }
        }

        return isSuccess;
    }

    bool AssetManager::AddMusicFromMemory(const std::string& musicName, const unsigned char* musicData, size_t dataSize)
    {
        if (m_mapMusic.contains(musicName))
        {
            F_ERROR("Failed to add music '{0}': Already exist!", musicName);
            return false;
        }

        SDL_RWops* rw = SDL_RWFromMem((void*)musicData, static_cast<int>(dataSize));
        Mix_MusicType type = DetectAudioFormat(musicData, dataSize);

        if (type == MUS_NONE)
        {
            F_ERROR("Failed to add music '{}' from memory. Unable to determine musc type", musicName);
            return false;
        }

        auto music = Mix_LoadMUSType_RW(rw, type, 1);
        if (!music)
        {
            F_ERROR("Failed to add music '{}' from memory", musicName);
            return false;
        }

        SoundParams params{ .name = musicName, .filename = "From Data", .duration = Mix_MusicDuration(music)};

        auto musicPtr = std::make_shared<Music>(params, MusicPtr{ music });
        if (!musicPtr)
        {
            F_ERROR("Failed to create music ptr for '{0}'", musicName);
            return false;
        }

        auto [itr, isSuccess] = m_mapMusic.emplace(musicName, std::move(musicPtr));

        return isSuccess;
    }

    std::shared_ptr<Music> AssetManager::GetMusic(const std::string& musicName)
    {
        auto musicItr = m_mapMusic.find(musicName);
        if (musicItr == m_mapMusic.end())
        {
            F_ERROR("Failed to get '{0}': Does not exist!", musicName);
            return nullptr;
        }

        return musicItr->second;
    }

    Mix_MusicType AssetManager::DetectAudioFormat(const unsigned char* audioData, size_t dataSize)
    {
        if (!audioData || dataSize < 12)
        {
            F_ERROR("Failed to detect the audio format. Data or size is invalid");
            return MUS_NONE;
        }

        // WAV Format
        if (std::memcmp(audioData, "RIFF", 4) == 0 &&
            std::memcmp(audioData + 8, "WAVE", 4) == 0)
        {
            return MUS_WAV;
        }

        // MP3 Format
        if (std::memcmp(audioData, "ID3", 3) == 0 ||
            audioData[0] == 0xFF && (audioData[1] & 0xE0) == 0xE0)
        {
            return MUS_MP3;
        }

        // OGG Format
        if (std::memcmp(audioData, "OggS", 4) == 0)
        {
            return MUS_OGG;
        }

        // Flac Format
        if (std::memcmp(audioData, "fLaC", 4) == 0)
        {
            return MUS_FLAC;
        }

        // Opus Format
        if (dataSize >= 36 &&
            std::memcmp(audioData + 28, "OpusHead", 8) == 0)
        {
            return MUS_OPUS;
        }

        F_ERROR("Failed to detect audio type: Unknown or unupported format (supported formats: WAVE, MP3, OGG, Flac, Opus)");

        return MUS_NONE;
    }

    bool AssetManager::AddSoundFx(const std::string& soundFxName, const std::string& filepath)
    {
        if (m_mapSoundFX.contains(soundFxName))
        {
            F_ERROR("Failed to add sound effect '{0}': Already exist!", soundFxName);
            return false;
        }

        Mix_Chunk* chunk = Mix_LoadWAV(filepath.c_str());
        if (!chunk)
        {
            std::string error{ Mix_GetError() };
            F_ERROR("Failed to load '{0}' at path '{1}': Mixer Error - {2}", soundFxName, filepath, error);
            return false;
        }

        SoundParams params{ .name = soundFxName, .filename = filepath, .duration = chunk->alen / 179.4 };

        auto pSoundFx = std::make_shared<SoundFX>(params, SoundFXPtr{ chunk });
        auto [itr, isSuccess] = m_mapSoundFX.emplace(soundFxName, std::move(pSoundFx));

        if (isSuccess)
        {
            std::lock_guard lock{ m_AssetMutex };
            fs::path path{ filepath };
            auto lastWrite = fs::last_write_time(path);
            if (CheckContainsValue(m_FilewatchParams, [&](const auto& params) { return params.filepath == filepath; }))
            {
                m_FilewatchParams.emplace_back(
                    AssetWatchParams{ .assetName = soundFxName,
                                      .filepath = filepath,
                                      .lastWrite = lastWrite,
                                      .type = AssetType::SOUNDFX });
            }
        }

        return isSuccess;
    }

    bool AssetManager::AddSoundFxFromMemory(const std::string& soundFxName, const unsigned char* soundFxData, size_t dataSize)
    {
        if (m_mapSoundFX.contains(soundFxName))
        {
            F_ERROR("Failed to add sound effect '{0}': Already exist!", soundFxName);
            return false;
        }

        SDL_RWops* rw = SDL_RWFromMem((void*)soundFxData, static_cast<int>(dataSize));
        auto chunk = Mix_LoadWAV_RW(rw, 1);
        if (!chunk)
        {
            F_ERROR("Failed to add soundFx '{}' from memory", soundFxName);
            return false;
        }

        SoundParams params{ .name = soundFxName, .filename = "From Data", .duration = chunk->alen / 179.4};

        auto soundFx = std::make_shared<SoundFX>(params, SoundFXPtr{ chunk });
        auto [itr, isSuccess] = m_mapSoundFX.emplace(soundFxName, std::move(soundFx));

        return isSuccess;
    }

    std::string AssetManager::GetAssetFilepath(const std::string& assetName, AssetType assetType)
    {
        switch (assetType)
        {
            case AssetType::TEXTURE:
            {
                auto itr = m_mapTextures.find(assetName);
                return itr != m_mapTextures.end() ? itr->second->GetPath() : std::string{};
            }
            case AssetType::FONT:
            {
                auto itr = m_mapFonts.find(assetName);
                return itr != m_mapFonts.end() ? itr->second->GetFilename() : std::string{};
            }
            case AssetType::SOUNDFX:
            {
                auto itr = m_mapSoundFX.find(assetName);
                return itr != m_mapSoundFX.end() ? itr->second->GetFilename() : std::string{};
            }
            case AssetType::MUSIC:
            {
                auto itr = m_mapMusic.find(assetName);
                return itr != m_mapMusic.end() ? itr->second->GetFilename() : std::string{};
            }
            case AssetType::PREFAB:
            {
                auto itr = m_mapPrefabs.find(assetName);
                return itr != m_mapPrefabs.end() ? itr->second->GetFilepath() : std::string{};
            }
        }

        return {};
    }

    std::shared_ptr<SoundFX> AssetManager::GetSoundFx(const std::string& soundFxName)
    {
        auto soundItr = m_mapSoundFX.find(soundFxName);
        if (soundItr == m_mapSoundFX.end())
        {
            F_ERROR("Failed to get sound effect '{0}': Does not exist!", soundFxName);
            return nullptr;
        }

        return soundItr->second;
    }

    bool AssetManager::AddPrefab(const std::string& prefabName, std::shared_ptr<Prefab> prefab)
    {
        if (m_mapPrefabs.contains(prefabName))
        {
            F_ERROR("Failed to add prefab '{}': Already exists in AssetManager", prefabName);
            return false;
        }

        auto [itr, bSuccess] = m_mapPrefabs.emplace(prefabName, std::move(prefab));
        return bSuccess;
    }

    std::shared_ptr<Prefab> AssetManager::GetPrefab(const std::string& prefabName)
    {
        auto prefabItr = m_mapPrefabs.find(prefabName);
        if (prefabItr == m_mapPrefabs.end())
        {
            F_ERROR("Failed to get Prefab '{}': Does not exist", prefabName);
            return nullptr;
        }

        return prefabItr->second;
    }

#ifdef IN_FEATHER_EDITOR

    bool AssetManager::AddCursor(const std::string& cursorName, const std::string& cursorPath)
    {
        F_FATAL("Not yet implemented");
        return false;
    }

    bool AssetManager::AddCursorFromMemory(const std::string& cursorName, unsigned char* cursorData, size_t dataSize)
    {
        if (m_mapCursors.contains(cursorName))
        {
            F_ERROR("Failed to add Cursor '{}': Already exists", cursorName);
            return false;
        }

        SDL_RWops* rw = SDL_RWFromConstMem(cursorData, static_cast<int>(dataSize));
        if (!rw)
        {
            F_ERROR("Failed to add cursor: {}", SDL_GetError());
            return false;
        }

        SDL_Surface* surface = IMG_Load_RW(rw, 1); // 1 = Automatically closes RWops
        if (!surface)
        {
            F_ERROR("Failed to add cursor: {}", IMG_GetError());
            return false;
        }

        SDL_Cursor* cursor = SDL_CreateColorCursor(surface, surface->w / 2, surface->h / 2);

        if (!cursor)
        {
            F_ERROR("Failed to add cursor. '{}'", SDL_GetError());
            return false;
        }

        SDL_FreeSurface(surface);

        return m_mapCursors.emplace(cursorName, MakeSharedFromSDLType<Cursor>(cursor)).second;
    }

    SDL_Cursor* AssetManager::GetCursor(const std::string& cursorName)
    {
        auto cursorItr = m_mapCursors.find(cursorName);
        if (cursorItr == m_mapCursors.end())
        {
            F_ERROR("Failed to get cursor '{}': Does not exist", cursorName);
            return nullptr;
        }

        return cursorItr->second.get();
    }

#endif

    std::vector<std::string> AssetManager::GetAssetKeyNames(AssetType assetType) const
    {
        switch (assetType)
        {
        case AssetType::TEXTURE:
            return GetKeys(m_mapTextures, [](const auto& pair) { return !pair.second->IsEditorTexture(); });
        case AssetType::FONT:
            return GetKeys(m_mapFonts);
        case AssetType::SOUNDFX:
            return GetKeys(m_mapSoundFX);
        case AssetType::MUSIC:
            return GetKeys(m_mapMusic);
        case AssetType::PREFAB:
            return GetKeys(m_mapPrefabs);
        default:
            F_ASSERT(false && "Cannot get this type!");
        }

        return std::vector<std::string>{};
    }

    bool AssetManager::ChangeAssetName(const std::string& oldName, const std::string& newName, AssetType assetType)
    {
        bool isSuccess{ false };

        switch (assetType)
        {
        case AssetType::TEXTURE:
            isSuccess = KeyChange(m_mapTextures, oldName, newName); break;
        case AssetType::FONT:
            isSuccess = KeyChange(m_mapFonts, oldName, newName); break;
        case AssetType::SOUNDFX:
            isSuccess = KeyChange(m_mapSoundFX, oldName, newName); break;
        case AssetType::MUSIC:
            isSuccess = KeyChange(m_mapMusic, oldName, newName); break;
        default:
            F_ASSERT(false && "Cannot get this type!"); break;
        }

        // If we are using the filewatcher, we need to also ensure to adjust the name
        if (m_FileWatcherRunning && isSuccess)
        {
            std::lock_guard lock{ m_AssetMutex };
            auto fileItr = std::ranges::find_if(m_FilewatchParams, [&](const auto& param) { return param.assetName == oldName; });

            if (fileItr != m_FilewatchParams.end())
            {
                fileItr->assetName = newName;
            }
        }

        return isSuccess;
    }

    bool AssetManager::HasAsset(const std::string& assetName, AssetType assetType)
    {
        switch (assetType)
        {
        case AssetType::TEXTURE:
            return m_mapTextures.contains(assetName);
        case AssetType::FONT:
            return m_mapFonts.contains(assetName);
        case AssetType::SOUNDFX:
            return m_mapSoundFX.contains(assetName);
        case AssetType::MUSIC:
            return m_mapMusic.contains(assetName);
        case AssetType::PREFAB:
            return m_mapPrefabs.contains(assetName);
        default:
            F_ASSERT(false && "Cannot get this type!");
        }

        return false;
    }

    bool AssetManager::DeleteAsset(const std::string& assetName, AssetType assetType)
    {
        bool isSuccess{ false };

        switch (assetType)
        {
        case AssetType::TEXTURE:
            isSuccess = std::erase_if(m_mapTextures, [&](const auto& pair) { return pair.first == assetName; }) > 0; break;
        case AssetType::FONT:
            isSuccess = std::erase_if(m_mapFonts, [&](const auto& pair) { return pair.first == assetName; }) > 0; break;
        case AssetType::SOUNDFX:
            isSuccess = std::erase_if(m_mapSoundFX, [&](const auto& pair) { return pair.first == assetName; }) > 0; break;
        case AssetType::MUSIC:
            isSuccess = std::erase_if(m_mapMusic, [&](const auto& pair) { return pair.first == assetName; }) > 0; break;
        case AssetType::PREFAB:
        {
            // Prefabs contain files that must be cleaned up
            if (auto pPrefab = GetPrefab(assetName))
            {
                if (!PrefabCreator::DeletePrefab(*pPrefab))
                {
                    F_ERROR("Failed to delete prefab '{}'", assetName);
                    return false;
                }

                isSuccess = m_mapPrefabs.erase(assetName) > 0;
                break;
            }

            F_ERROR("Failed to delete prefab '{}' - Does not exist in asset manager", assetName);
            return false;
        }
        default:
            F_ASSERT(false && "Cannot get this type!");
        }

        // If the file watcher is enabled, we need to remove the file from being watched
        if (m_FileWatcherRunning && isSuccess)
        {
            std::lock_guard lock{ m_AssetMutex };
            bool erased = std::erase_if(m_FilewatchParams, [&](const auto& param) { return param.assetName == assetName; }) > 0;

            if (!erased)
            {
                F_WARN("Failed to erase '{}' from File Watcher Params: Must not be present", assetName);
                // Non-fatal error
            }
        }

        if (isSuccess)
        {
            F_TRACE("Deleted asset '{}'", assetName);
        }

        return isSuccess;
    }

    bool AssetManager::DeleteAssetFromPath(const std::string& assetPath)
    {
        auto textureItr = std::ranges::find_if(m_mapTextures, [&](const auto& pair) { return pair.second->GetPath() == assetPath; });

        if (textureItr != m_mapTextures.end())
        {
            std::string sTextureName{ textureItr->first };
            return DeleteAsset(sTextureName, AssetType::TEXTURE);
        }

        auto musicItr = std::ranges::find_if(m_mapMusic, [&](const auto& pair) { return pair.second->GetFilename() == assetPath; });

        if (musicItr != m_mapMusic.end())
        {
            std::string sMusicName{ musicItr->first };
            return DeleteAsset(sMusicName, AssetType::MUSIC);
        }

        auto soundItr = std::ranges::find_if(m_mapSoundFX, [&](const auto& pair) { return pair.second->GetFilename() == assetPath; });

        if (soundItr != m_mapSoundFX.end())
        {
            std::string sSoundName{ soundItr->first };
            return DeleteAsset(sSoundName, AssetType::SOUNDFX);
        }

        return true;
    }

    void AssetManager::CreateLuaAssetManager(sol::state& lua)
    {
        auto& mainRegistry = MAIN_REGISTRY();
        auto& asset_manager = mainRegistry.GetAssetManager();

        lua.new_usertype<AssetManager>(
            "AssetManager",
            sol::no_constructor,
            "addTexture",
            sol::overload(
                [&](const std::string& assetName, const std::string& filepath, bool pixel_art)
                {
                    return asset_manager.AddTexture(assetName, filepath, pixel_art, false);
                },
                [&](const std::string& assetName, const std::string& filepath, bool pixel_art, bool isTileset)
                {
                    return asset_manager.AddTexture(assetName, filepath, pixel_art, isTileset);
                }
            ),
            "addMusic",
            [&](const std::string& musicName, const std::string& filepath)
            {
                return asset_manager.AddMusic(musicName, filepath);
            },
            "addSound",
            [&](const std::string& soundFxName, const std::string& filepath)
            {
                return asset_manager.AddSoundFx(soundFxName, filepath);
            },
            "addFont",
            [&](const std::string& fontName, const std::string& fontPath, float fontSize)
            {
                return asset_manager.AddFont(fontName, fontPath, fontSize);
            }
        );
    }

    void AssetManager::Update()
    {
        std::shared_lock sharedLock{ m_AssetMutex };
        auto dirtyView = m_FilewatchParams | std::views::filter([](const auto& param) { return param.isDirty; });

        if (!dirtyView.empty())
        {
            sharedLock.unlock();
            std::unique_lock lock{ m_AssetMutex };
            for (auto& param : dirtyView)
            {
                ReloadAsset(param);
                param.isDirty = false;
            }
        }
    }

    void AssetManager::FileWatcher()
    {
        while (m_FileWatcherRunning)
        {
            std::this_thread::sleep_for(2s);

            for (auto& fileParam : m_FilewatchParams)
            {
                std::shared_lock sharedLock{ m_AssetMutex };
                fs::path path{ fileParam.filepath };
                if (!fs::exists(path))
                    continue;

                if (fileParam.lastWrite != fs::last_write_time(path))
                {
                    sharedLock.unlock();
                    std::unique_lock lock{ m_AssetMutex };
                    fileParam.isDirty = true;
                }
            }
        }
    }

    void AssetManager::ReloadAsset(const AssetWatchParams& assetParams)
    {
        switch (assetParams.type)
        {
        case AssetType::TEXTURE: ReloadTexture(assetParams.assetName); break;
        case AssetType::FONT: ReloadFont(assetParams.assetName); break;
        case AssetType::SHADER: ReloadShader(assetParams.assetName); break;
        case AssetType::MUSIC: ReloadMusic(assetParams.assetName); break;
        case AssetType::SOUNDFX: ReloadSoundFx(assetParams.assetName); break;
        }
    }

    void AssetManager::ReloadTexture(const std::string& textureName)
    {
        auto fileParamItr = std::ranges::find_if(m_FilewatchParams, [&](const auto& param) { return param.assetName == textureName; });

        if (fileParamItr == m_FilewatchParams.end())
        {
            F_ERROR("Trying to reload a texture that has not been loaded?");
            return;
        }

        // We are assuming that the texture is in the map.
        // Could potentially cause a crash, will look more into this.
        auto& pTexture = m_mapTextures[textureName];

        fileParamItr->lastWrite = fs::last_write_time(fs::path{ pTexture->GetPath() });
        // Delete the old texture and then reload
        auto id = pTexture->GetID();
        glDeleteTextures(1, &id);

        auto pNewTexture = TextureLoader::Create(pTexture->GetType(), pTexture->GetPath(), pTexture->IsTileset());

        pTexture = pNewTexture;
        F_TRACE("Reloaded texture: {}", textureName);
    }

    void AssetManager::ReloadSoundFx(const std::string& soundName)
    {
        auto fileParamItr = std::ranges::find_if(m_FilewatchParams, [&](const auto& param) { return param.assetName == soundName; });

        if (fileParamItr == m_FilewatchParams.end())
        {
            F_ERROR("Trying to reload a texture that has not been loaded?");
            return;
        }

        fileParamItr->lastWrite = fs::last_write_time(fs::path{ fileParamItr->filepath });

        if (!DeleteAsset(soundName, AssetType::SOUNDFX))
        {
            F_ERROR("Failed to reload SoundFx: {}", soundName);
            return;
        }

        if (!AddSoundFx(soundName, fileParamItr->filepath))
        {
            F_ERROR("Failed to reload SoundFx: {}", soundName);
            return;
        }

        F_TRACE("Reloaded SoundFx: {}", soundName);
    }

    void AssetManager::ReloadMusic(const std::string& musicName)
    {
        auto fileParamItr = std::ranges::find_if(m_FilewatchParams, [&](const auto& param) { return param.assetName == musicName; });

        if (fileParamItr == m_FilewatchParams.end())
        {
            F_ERROR("Trying to music that has not been loaded?");
            return;
        }

        fileParamItr->lastWrite = fs::last_write_time(fs::path{ fileParamItr->filepath });

        if (!DeleteAsset(musicName, AssetType::MUSIC))
        {
            F_ERROR("Failed to reload SoundFx: {}", musicName);
            return;
        }

        if (!AddMusic(musicName, fileParamItr->filepath))
        {
            F_ERROR("Failed to reload SoundFx: {}", musicName);
            return;
        }

        F_TRACE("Reloaded Music: {}", musicName);
    }

    void AssetManager::ReloadFont(const std::string& fontName)
    {
        auto fileParamItr = std::ranges::find_if(m_FilewatchParams, [&](const auto& param) { return param.assetName == fontName; });

        if (fileParamItr == m_FilewatchParams.end())
        {
            F_ERROR("Trying to music that has not been loaded?");
            return;
        }

        fileParamItr->lastWrite = fs::last_write_time(fs::path{ fileParamItr->filepath });

        auto& pFont = m_mapFonts[fontName];
        float fontSize = pFont->GetFontSize();

        if (!DeleteAsset(fontName, AssetType::FONT))
        {
            F_ERROR("Failed to reload SoundFx: {}", fontName);
            return;
        }

        if (!AddFont(fontName, fileParamItr->filepath, fontSize))
        {
            F_ERROR("Failed to reload SoundFx: {}", fontName);
            return;
        }

        F_TRACE("Reloaded Font: {}", fontName);
    }

    void AssetManager::ReloadShader(const std::string& shaderName)
    {
        // TODO:
    }

}
