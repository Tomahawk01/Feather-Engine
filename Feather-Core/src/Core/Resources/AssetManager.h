#pragma once

#include "Renderer/Essentials/Shader.h"
#include "Renderer/Essentials/Texture.h"

#include <map>
#include <memory>
#include <string>

namespace Feather {
	
	class AssetManager
	{
	public:
		AssetManager() = default;
		~AssetManager() = default;

		bool AddTexure(const std::string& textureName, const std::string& texturePath, bool pixelArt = true);
		const Feather::Texture& GetTexture(const std::string& textureName);

		bool AddShader(const std::string& shaderName, const std::string& vertexPath, const std::string& fragmentPath);
		Feather::Shader& GetShader(const std::string& shaderName);

	private:
		std::map<std::string, std::shared_ptr<Feather::Texture>> m_mapTextures{};
		std::map<std::string, std::shared_ptr<Feather::Shader>> m_mapShaders{};
	};

}
