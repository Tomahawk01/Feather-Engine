#pragma once

namespace fs = std::filesystem;

namespace Feather {

	class Texture;

	struct AudioConfigInfo
	{
		using SoundChannelMap = std::map<int, std::pair<bool, int>>;

		/* Overrides all sound and music channels */
		bool globalOverrideEnabled{ false };
		/* The amount to override all channels. Percentage [0 - 100] */
		int globalVolumeOverride{ 100 };
		/* Overrides the music channel. This is overridden if global override is enabled */
		bool musicVolumeOverrideEnabled{ false };
		/* The amount to override the music channel. Percentage [0 - 100] */
		int musicVolumeOverride{ 100 };

		bool UpdateSoundChannels(int numChannels);
		bool EnableChannelOverride(int channel, bool enable);
		bool SetChannelVolume(int channel, int volume);

		const SoundChannelMap& GetSoundChannelMap() const { return mapSoundChannelVolume; }
		const int GetAllocatedChannelCount() const { return allocatedSoundChannels; }

	private:
		/* @brief Adds more channels to the channel map */
		void AddChannels(int numChannels);
		void RemoveChannels(int numChannels);

	private:
		int allocatedSoundChannels{ 8 };

		/* Map of the allocated channels. Channel to enabled/volume override */
		SoundChannelMap mapSoundChannelVolume{
			{ 0, { false, 100 } },
			{ 1, { false, 100 } },
			{ 2, { false, 100 } },
			{ 3, { false, 100 } },
			{ 4, { false, 100 } },
			{ 5, { false, 100 } },
			{ 6, { false, 100 } },
			{ 7, { false, 100 } }
		};
	};

	/*
	* @brief Enum class defining the different folder types within a project
	*/
	enum class EProjectFolderType
	{
		Content,			// Root content folder (e.g., for user assets)
		Config,				// General configuration files
		GameConfig,			// Game-specific configuration files
		EditorConfig,		// Editor-specific settings and configuration files
		Assets,				// Folder containing all asset types
		Scripts,			// Game lua scripts
		Textures,			// Texture files (e.g. PNG, JPG)
		Shaders,			// User GLSL shader files
		Prefabs,			// Prefab definitions (Serialized Prefabbed entitites)
		Fonts,				// Font files use in the game.
		Scenes,				// Scene or level serialized files
		SoundFx,			// Sound Fx files
		Music,				// Background music files
	};

	class ProjectInfo
	{
	public:
		ProjectInfo() = default;
		~ProjectInfo() = default;

		void SetProjectPath(const fs::path& path);
		const fs::path& GetProjectPath() const;

		void SetProjectFilePath(const fs::path& path);
		std::optional<fs::path> GetProjectFilePath() const;

		void SetMainLuaScriptPath(const fs::path& path);
		std::optional<fs::path> GetMainLuaScriptPath() const;

		void SetScriptListPath(const fs::path& path);
		std::optional<fs::path> GetScriptListPath() const;

		void SetFileIconPath(const fs::path& path);
		std::optional<fs::path> GetFileIconPath() const;

		bool AddFolderPath(EProjectFolderType folderType, const fs::path& path);

		std::optional<fs::path> TryGetFolderPath(EProjectFolderType folderType);

		inline const Texture* GetIconTexturePtr() const { return m_IconTexture.get(); }

		inline void SetProjectName(const std::string& projectName) { m_ProjectName = projectName; }
		inline const std::string& GetProjectName() const { return m_ProjectName; }
		inline void SetProjectVersion(const std::string& version) { m_ProjectVersion = version; }
		inline const std::string& GetProjectVersion() const { return m_ProjectVersion; }
		inline void SetProjectDescription(const std::string& description) { m_ProjectDescription = description; }
		inline const std::string& GetProjectDescription() const { return m_ProjectDescription; }
		inline void SetCopyRightNotice(const std::string& copyRightNotice) { m_CopyRightNotice = copyRightNotice; }
		inline const std::string& GetCopyRightNotice() const { return m_CopyRightNotice; }
		inline const std::string& GetDefaultScene() const { return m_DefaultScene; }
		inline void SetDefaultScene(const std::string& defaultScene) { m_DefaultScene = defaultScene; }
		inline AudioConfigInfo& GetAudioConfig() { return m_AudioConfig; }
		inline const AudioConfigInfo& GetAudioConfig() const { return m_AudioConfig; }

		inline const std::unordered_map<EProjectFolderType, fs::path>& GetProjectPaths() const { return m_mapProjectFolderPaths; }

	private:
		std::unordered_map<EProjectFolderType, fs::path> m_mapProjectFolderPaths;
		fs::path m_ProjectPath{};
		std::optional<fs::path> m_ProjectFilePath{ std::nullopt };
		std::optional<fs::path> m_MainLuaScript{ std::nullopt };
		std::optional<fs::path> m_IconFilePath{ std::nullopt };
		std::optional<fs::path> m_ScriptListPath{ std::nullopt };
		std::string m_DefaultScene{};
		std::string m_ProjectName{};
		std::string m_ProjectVersion{ "1.0.0.0" };
		std::string m_ProjectDescription{};
		std::string m_CopyRightNotice{};
		std::shared_ptr<Texture> m_IconTexture{ nullptr };

		AudioConfigInfo m_AudioConfig{};

		bool m_UseVSync{ true };
		bool m_Resizable{ false };
		bool m_FullScreen{ false };
		bool m_Borderless{ false };
		bool m_AllowHighDPI{ false };
	};

	using ProjectInfoPtr = std::shared_ptr<ProjectInfo>;

	struct GameConfig
	{
		std::string gameName{};
		std::string startupScene{};

		int windowWidth{ 640 };
		int windowHeight{ 480 };
		uint32_t windowFlags{ 0 };

		int cameraWidth{ 640 };
		int cameraHeight{ 480 };
		float cameraScale{ 1.0f };

		bool physicsEnabled{ false };
		int32_t positionIterations{ 8 };
		int32_t velocityIterations{ 8 };
		float gravity{ 9.8f };

		bool packageAssets{ false };

		void Reset()
		{
			gameName.clear();
			startupScene.clear();

			windowWidth = 640;
			windowHeight = 480;
			windowFlags = 0;

			cameraWidth = 640;
			cameraHeight = 480;
			cameraScale = 1.0f;

			physicsEnabled = false;
			positionIterations = 8;
			velocityIterations = 8;
			gravity = 9.8f;

			packageAssets = false;
		}
	};

}
