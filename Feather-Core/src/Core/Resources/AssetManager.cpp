#include "AssetManager.h"

#include "Logger/Logger.h"
#include "Renderer/Essentials/TextureLoader.h"
#include "Renderer/Essentials/ShaderLoader.h"

namespace Feather {

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

    const Texture& AssetManager::GetTexture(const std::string& textureName)
    {
        auto texIter = m_mapTextures.find(textureName);
        if (texIter == m_mapTextures.end())
        {
            F_ERROR("Failed to get texture '{0}': Does not exist!", textureName);
            return Texture();
        }

        return *texIter->second;
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

    Shader& AssetManager::GetShader(const std::string& shaderName)
    {
        auto shaderIter = m_mapShaders.find(shaderName);
        if (shaderIter == m_mapShaders.end())
        {
            F_ERROR("Failed to get shader '{0}': Does not exist!", shaderName);
            Shader shader{};
            return shader;
        }

        return *shaderIter->second;
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
            }
        );
    }

}
