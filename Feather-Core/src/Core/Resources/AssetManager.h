#pragma once

#include <sol/sol.hpp>
#include <SDL_mixer.h>

using Cursor = std::shared_ptr<struct SDL_Cursor>;

namespace Feather {
	
	enum class AssetType;

	class Prefab;
	class Texture;
	class Shader;
	class Font;
	class Music;
	class SoundFX;

	class AssetManager
	{
	public:
		AssetManager(bool enableFilewatcher = false);
		~AssetManager();

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
		bool AddMusicFromMemory(const std::string& musicName, const unsigned char* musicData, size_t dataSize);
		std::shared_ptr<Music> GetMusic(const std::string& musicName);

		Mix_MusicType DetectAudioFormat(const unsigned char* audioData, size_t dataSize);

		bool AddSoundFx(const std::string& soundFxName, const std::string& filepath);
		bool AddSoundFxFromMemory(const std::string& soundFxName, const unsigned char* soundFxData, size_t dataSize);
		std::shared_ptr<SoundFX> GetSoundFx(const std::string& soundFxName);
		
		bool AddPrefab(const std::string& prefabName, std::shared_ptr<Prefab> prefab);

		std::shared_ptr<Prefab> GetPrefab(const std::string& prefabName);

#ifdef IN_FEATHER_EDITOR
		bool AddCursor(const std::string& cursorName, const std::string& cursorPath);
		bool AddCursorFromMemory(const std::string& cursorName, unsigned char* cursorData, size_t dataSize);

		SDL_Cursor* GetCursor(const std::string& cursorName);
#endif

		inline const std::map<std::string, std::shared_ptr<Texture>>& GetAllTextures() const { return m_mapTextures; }
		inline const std::map<std::string, std::shared_ptr<SoundFX>>& GetAllSoundFx() const { return m_mapSoundFX; }
		inline const std::map<std::string, std::shared_ptr<Shader>>& GetAllShaders() const { return m_mapShaders; }
		inline const std::map<std::string, std::shared_ptr<Font>>& GetAllFonts() const { return m_mapFonts; }
		inline const std::map<std::string, std::shared_ptr<Music>>& GetAllMusic() const { return m_mapMusic; }
		inline const std::map<std::string, std::shared_ptr<Prefab>>& GetAllPrefabs() const { return m_mapPrefabs; }

		std::vector<std::string> GetAssetKeyNames(AssetType assetType) const;

		bool ChangeAssetName(const std::string& oldName, const std::string& newName, AssetType assetType);
		bool HasAsset(const std::string& assetName, AssetType assetType);
		bool DeleteAsset(const std::string& assetName, AssetType assetType);
		bool DeleteAssetFromPath(const std::string& assetPath);

		static void CreateLuaAssetManager(sol::state& lua);

		void Update();

	private:
		void FileWatcher();

		struct AssetWatchParams
		{
			std::string assetName{};
			std::string filepath{};
			std::filesystem::file_time_type lastWrite;
			AssetType type{};
			bool isDirty{ false };
		};

		void ReloadAsset(const AssetWatchParams& assetParams);

		void ReloadTexture(const std::string& textureName);
		void ReloadSoundFx(const std::string& soundName);
		void ReloadMusic(const std::string& musicName);
		void ReloadFont(const std::string& fontName);
		void ReloadShader(const std::string& shaderName);

	private:
		std::map<std::string, std::shared_ptr<Texture>> m_mapTextures{};
		std::map<std::string, std::shared_ptr<Shader>> m_mapShaders{};
		std::map<std::string, std::shared_ptr<Font>> m_mapFonts{};

		std::map<std::string, std::shared_ptr<Music>> m_mapMusic{};
		std::map<std::string, std::shared_ptr<SoundFX>> m_mapSoundFX{};
		std::map<std::string, std::shared_ptr<Prefab>> m_mapPrefabs{};

#ifdef IN_FEATHER_EDITOR
		std::map<std::string, Cursor> m_mapCursors;
#endif

		std::vector<AssetWatchParams> m_FilewatchParams;

		std::atomic<bool> m_FileWatcherRunning;
		std::jthread m_WatchThread;
		std::mutex m_CallbackMutex;
		std::shared_mutex m_AssetMutex;
	};

}
