#pragma once

#include <map>
#include <memory>
#include <string>
#include <sol/sol.hpp>

#include "Renderer/Essentials/Shader.h"
#include "Renderer/Essentials/Texture.h"
#include "Renderer/Essentials/Font.h"
#include "Core/ECS/Registry.h"
#include "Sounds/Essentials/Music.h"
#include "Sounds/Essentials/SoundFX.h"

namespace Feather {
	
	class AssetManager
	{
	public:
		AssetManager() = default;
		~AssetManager() = default;

		bool AddTexure(const std::string& textureName, const std::string& texturePath, bool pixelArt = true);
		bool AddTextureFromMemory(const std::string& textureName, const unsigned char* imageData, size_t length, bool pixelArt = true);
		std::shared_ptr<Texture> GetTexture(const std::string& textureName);

		bool AddFont(const std::string& fontName, const std::string& fontPath, float fontSize = 32.0f);
		bool AddFontFromMemory(const std::string& fontName, unsigned char* fontData, float fontSize = 32.0f);
		std::shared_ptr<Font> GetFont(const std::string& fontName);

		bool AddShader(const std::string& shaderName, const std::string& vertexPath, const std::string& fragmentPath);
		bool AddShaderFromMemory(const std::string& shaderName, const char* vertexShader, const char* fragmentShader);
		std::shared_ptr<Shader> GetShader(const std::string& shaderName);

		bool AddMusic(const std::string& musicName, const std::string& filepath);
		std::shared_ptr<Music> GetMusic(const std::string& musicName);

		bool AddSoundFx(const std::string& soundFxName, const std::string& filepath);
		std::shared_ptr<SoundFX> GetSoundFx(const std::string& soundFxName);

		static void CreateLuaAssetManager(sol::state& lua, Registry& registry);

	private:
		std::map<std::string, std::shared_ptr<Texture>> m_mapTextures{};
		std::map<std::string, std::shared_ptr<Shader>> m_mapShaders{};
		std::map<std::string, std::shared_ptr<Font>> m_mapFonts{};

		std::map<std::string, std::shared_ptr<Music>> m_mapMusic{};
		std::map<std::string, std::shared_ptr<SoundFX>> m_mapSoundFX{};
	};

}
