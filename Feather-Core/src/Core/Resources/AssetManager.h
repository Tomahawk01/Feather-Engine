#pragma once

#include <map>
#include <memory>
#include <string>
#include <sol/sol.hpp>

#include "Renderer/Essentials/Shader.h"
#include "Renderer/Essentials/Texture.h"
#include "Core/ECS/Registry.h"
#include "Sounds/Essentials/Music.h"

namespace Feather {
	
	class AssetManager
	{
	public:
		AssetManager() = default;
		~AssetManager() = default;

		bool AddTexure(const std::string& textureName, const std::string& texturePath, bool pixelArt = true);
		const Texture& GetTexture(const std::string& textureName);

		bool AddShader(const std::string& shaderName, const std::string& vertexPath, const std::string& fragmentPath);
		Shader& GetShader(const std::string& shaderName);

		bool AddMusic(const std::string& musicName, const std::string& filepath);
		std::shared_ptr<Music> GetMusic(const std::string& musicName);

		static void CreateLuaAssetManager(sol::state& lua, Registry& registry);

	private:
		std::map<std::string, std::shared_ptr<Texture>> m_mapTextures{};
		std::map<std::string, std::shared_ptr<Shader>> m_mapShaders{};

		std::map<std::string, std::shared_ptr<Music>> m_mapMusic{};
	};

}
