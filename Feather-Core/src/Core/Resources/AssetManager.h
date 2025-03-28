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
	
	enum class AssetType;

	class AssetManager
	{
	public:
		AssetManager() = default;
		~AssetManager() = default;

		bool CreateDefaultFonts();

		bool AddTexture(const std::string& textureName, const std::string& texturePath, bool pixelArt = true, bool isTileset = false);
		bool AddTextureFromMemory(const std::string& textureName, const unsigned char* imageData, size_t length, bool pixelArt = true, bool isTileset = false);
		std::shared_ptr<Texture> GetTexture(const std::string& textureName);

		std::vector<std::string> GetTilesetNames() const;

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

		inline const std::map<std::string, std::shared_ptr<Texture>>& GetAllTextures() const { return m_mapTextures; }
		inline const std::map<std::string, std::shared_ptr<SoundFX>>& GetAllSoundFx() const { return m_mapSoundFX; }
		inline const std::map<std::string, std::shared_ptr<Shader>>& GetAllShaders() const { return m_mapShaders; }
		inline const std::map<std::string, std::shared_ptr<Font>>& GetAllFonts() const { return m_mapFonts; }
		inline const std::map<std::string, std::shared_ptr<Music>>& GetAllMusic() const { return m_mapMusic; }

		std::vector<std::string> GetAssetKeyNames(AssetType assetType) const;

		bool ChangeAssetName(const std::string& oldName, const std::string& newName, AssetType assetType);
		bool HasAsset(const std::string& assetName, AssetType assetType);
		bool DeleteAsset(const std::string& assetName, AssetType assetType);

		static void CreateLuaAssetManager(sol::state& lua);

	private:
		std::map<std::string, std::shared_ptr<Texture>> m_mapTextures{};
		std::map<std::string, std::shared_ptr<Shader>> m_mapShaders{};
		std::map<std::string, std::shared_ptr<Font>> m_mapFonts{};

		std::map<std::string, std::shared_ptr<Music>> m_mapMusic{};
		std::map<std::string, std::shared_ptr<SoundFX>> m_mapSoundFX{};
	};

}
