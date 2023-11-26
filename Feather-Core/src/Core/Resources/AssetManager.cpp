#include "AssetManager.h"

#include "Logger/Logger.h"
#include "Renderer/Essentials/TextureLoader.h"
#include "Renderer/Essentials/ShaderLoader.h"

namespace Feather {

    bool Feather::AssetManager::AddTexure(const std::string& textureName, const std::string& texturePath, bool pixelArt)
    {
        if (m_mapTextures.find(textureName) != m_mapTextures.end())
        {
            F_ERROR("Failed to add texture '{0}' - Already exists!", textureName);
            return false;
        }

        auto texture = std::move(Feather::TextureLoader::Create(
                pixelArt ? Feather::Texture::TextureType::PIXEL : Feather::Texture::TextureType::BLENDED,
                texturePath));
        if (!texture)
        {
            F_ERROR("Failed to load texture '{0}' at path '{1}'", textureName, texturePath);
            return false;
        }

        m_mapTextures.emplace(textureName, std::move(texture));

        return true;
    }

    const Feather::Texture& Feather::AssetManager::GetTexture(const std::string& textureName)
    {
        auto texIter = m_mapTextures.find(textureName);
        if (texIter == m_mapTextures.end())
        {
            F_ERROR("Failed to get texture '{0}' - Does not exist!", textureName);
            return Feather::Texture();
        }

        return *texIter->second;
    }

    bool Feather::AssetManager::AddShader(const std::string& shaderName, const std::string& vertexPath, const std::string& fragmentPath)
    {
        if (m_mapShaders.find(shaderName) != m_mapShaders.end())
        {
            F_ERROR("Failed to add shader '{0}' - Already exists!", shaderName);
            return false;
        }

        auto shader = std::move(Feather::ShaderLoader::Create(vertexPath, fragmentPath));
        if (!shader)
        {
            F_ERROR("Failed to load shader '{0}' at vert path '{1}' and frag path '{2}'", shaderName, vertexPath, fragmentPath);
            return false;
        }

        m_mapShaders.emplace(shaderName, std::move(shader));
        return true;
    }

    Feather::Shader& Feather::AssetManager::GetShader(const std::string& shaderName)
    {
        auto shaderIter = m_mapShaders.find(shaderName);
        if (shaderIter == m_mapShaders.end())
        {
            F_ERROR("Failed to get shader '{0}' - Does not exist!", shaderName);
            Feather::Shader shader{};
            return shader;
        }

        return *shaderIter->second;
    }

}
