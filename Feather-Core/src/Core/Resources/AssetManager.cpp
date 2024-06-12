#include "AssetManager.h"

#include "Logger/Logger.h"
#include "Renderer/Essentials/TextureLoader.h"
#include "Renderer/Essentials/ShaderLoader.h"
#include "Renderer/Essentials/FontLoader.h"

#include "Core/Resources/fonts/default_fonts.h"

namespace Feather {

    bool AssetManager::CreateDefaultFonts()
    {
        if (!AddFontFromMemory("defaultFont", pixel_font))
        {
            F_ERROR("Failed to create default pixel font");
            return false;
        }

        // TODO: Add more default fonts

        return true;
    }

    bool AssetManager::AddTexure(const std::string& textureName, const std::string& texturePath, bool pixelArt)
    {
        if (m_mapTextures.find(textureName) != m_mapTextures.end())
        {
            F_ERROR("Failed to add texture '{0}': Already exists!", textureName);
            return false;
        }

        auto texture = std::move(TextureLoader::Create(
                pixelArt ? Texture::TextureType::PIXEL : Texture::TextureType::BLENDED,
                texturePath));
        if (!texture)
        {
            F_ERROR("Failed to load texture '{0}' at path '{1}'", textureName, texturePath);
            return false;
        }

        m_mapTextures.emplace(textureName, std::move(texture));

        return true;
    }

    bool AssetManager::AddTextureFromMemory(const std::string& textureName, const unsigned char* imageData, size_t length, bool pixelArt)
    {
        if (m_mapTextures.contains(textureName))
        {
            F_ERROR("AssetManager: Texture '{}': Already exists!", textureName);
            return false;
        }

        auto texture = std::move(TextureLoader::CreateFromMemory(imageData, length));
        // Load texture
        if (!texture)
        {
            F_ERROR("Unable to load texture '{}' from memory", textureName);
            return false;
        }

        // Insert texture into the map
        m_mapTextures.emplace(textureName, std::move(texture));

        return true;
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

        m_mapFonts.emplace(fontName, std::move(pFont));

        return true;
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

        m_mapFonts.emplace(fontName, std::move(pFont));

        return true;
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
        if (m_mapShaders.find(shaderName) != m_mapShaders.end())
        {
            F_ERROR("Failed to add shader '{0}': Already exists!", shaderName);
            return false;
        }

        auto shader = std::move(ShaderLoader::Create(vertexPath, fragmentPath));
        if (!shader)
        {
            F_ERROR("Failed to load shader '{0}' at vert path '{1}' and frag path '{2}'", shaderName, vertexPath, fragmentPath);
            return false;
        }

        m_mapShaders.emplace(shaderName, std::move(shader));
        return true;
    }

    bool AssetManager::AddShaderFromMemory(const std::string& shaderName, const char* vertexShader, const char* fragmentShader)
    {
        if (m_mapShaders.find(shaderName) != m_mapShaders.end())
        {
            F_ERROR("Failed to add shader - '{0}' -- Already exists!", shaderName);
            return false;
        }

        auto shader = std::move(ShaderLoader::CreateFromMemory(vertexShader, fragmentShader));
        m_mapShaders.insert(std::make_pair(shaderName, std::move(shader)));

        return true;
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
        if (m_mapMusic.find(musicName) != m_mapMusic.end())
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

        m_mapMusic.emplace(musicName, std::move(musicPtr));

        return true;
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

    bool AssetManager::AddSoundFx(const std::string& soundFxName, const std::string& filepath)
    {
        if (m_mapSoundFX.find(soundFxName) != m_mapSoundFX.end())
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
        m_mapSoundFX.emplace(soundFxName, std::move(pSoundFx));

        return true;
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

    void AssetManager::CreateLuaAssetManager(sol::state& lua, Registry& registry)
    {
        auto& asset_manager = registry.GetContext<std::shared_ptr<AssetManager>>();
        if (!asset_manager)
        {
            F_ERROR("Failed to bind asset manager to lua: Does not exist in registry!");
            return;
        }

        lua.new_usertype<AssetManager>(
            "AssetManager",
            sol::no_constructor,
            "add_texture", [&](const std::string& assetName, const std::string& filepath, bool pixel_art)
            {
                return asset_manager->AddTexure(assetName, filepath, pixel_art);
            },
            "add_music", [&](const std::string& musicName, const std::string& filepath)
            {
                return asset_manager->AddMusic(musicName, filepath);
            },
            "add_sound", [&](const std::string& soundFxName, const std::string& filepath)
            {
                return asset_manager->AddSoundFx(soundFxName, filepath);
            },
            "add_font", [&](const std::string& fontName, const std::string& fontPath, float fontSize)
            {
                return asset_manager->AddFont(fontName, fontPath, fontSize);
            }
        );
    }

}
