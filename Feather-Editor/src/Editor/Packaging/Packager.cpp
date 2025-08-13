#include "Packager.h"

#include "ScriptCompiler.h"
#include "IconReplacer.h"
#include "AssetPackager.h"

#include "Logger/Logger.h"
#include "FileSystem/Serializers/LuaSerializer.h"
#include "Utils/HelperUtilities.h"
#include "Utils/ThreadPool.h"
#include "Core/CoreUtils/ProjectInfo.h"
#include "Editor/Scene/SceneObject.h"

#include <rapidjson/error/en.h>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

namespace Feather {

	Packager::Packager(std::unique_ptr<PackageData> data, std::shared_ptr<ThreadPool> threadPool)
		: m_PackageData{ std::move(data) }
		, m_Packaging{ false }
		, m_HasError{ false }
		, m_ThreadPool{ threadPool }
	{
		m_PackageThread = std::thread([this] { RunPackager(); });
	}

	Packager::~Packager()
	{
		FinishPackaging();
	}

	bool Packager::Completed() const
	{
		return !m_Packaging;
	}

	bool Packager::HasError() const
	{
		return m_HasError;
	}

	PackagingProgress Packager::GetProgress() const
	{
		std::shared_lock lock(m_ProgressMutex);
		return m_Progress;
	}

	void Packager::FinishPackaging()
	{
		try
		{
			if (fs::exists(m_PackageData->TempDataPath))
			{
				auto numFiles = fs::remove_all(fs::path{ m_PackageData->TempDataPath });
				if (numFiles > 0)
				{
					F_TRACE("Successfully deleted temp packaging files ({})", numFiles);
				}
			}
		}
		catch (const fs::filesystem_error& err)
		{
			F_ERROR("Failed to delete temp data at path '{}': {}", m_PackageData->TempDataPath, err.what());
		}

		if (m_PackageThread.joinable())
			m_PackageThread.join();
	}

	void Packager::RunPackager()
	{
	}

	void Packager::UpdateProgress(float percent, std::string_view message)
	{
		std::lock_guard lock(m_ProgressMutex);
		m_Progress.percent = percent;
		m_Progress.message = message;
	}

	std::string Packager::CreateConfigFile(const std::string& tempFilepath)
	{
		std::unique_ptr<LuaSerializer> serializer{ nullptr };
		if (!fs::exists(tempFilepath))
		{
			F_ERROR("Failed to create settings config file. '{}' does not exist", tempFilepath);
			return {};
		}

		const std::string settingsConfigPath = tempFilepath + PATH_SEPARATOR + "config.lua";

		try
		{
			serializer = std::make_unique<LuaSerializer>(settingsConfigPath);
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to create setting config file '{}': {}", settingsConfigPath, ex.what());
			return {};
		}

		serializer->StartNewTable("GameConfig")
			.AddKeyValuePair("GameName", m_PackageData->GameConfig->gameName, true, false, false, true)
			.AddKeyValuePair("StartupScene", m_PackageData->GameConfig->startupScene, true, false, false, true)
			.AddKeyValuePair("PackageAssets", m_PackageData->GameConfig->packageAssets ? "true" : "false")
			.StartNewTable("WindowParams")
			.AddKeyValuePair("width", m_PackageData->GameConfig->windowWidth)
			.AddKeyValuePair("height", m_PackageData->GameConfig->windowHeight)
			.AddKeyValuePair("flags", m_PackageData->GameConfig->windowFlags)
			.EndTable() // WindowParams
			.StartNewTable("PhysicsParams")
			.AddKeyValuePair("enabled", m_PackageData->GameConfig->physicsEnabled ? "true" : "false")
			.AddKeyValuePair("positionIterations", m_PackageData->GameConfig->positionIterations)
			.AddKeyValuePair("velocityIterations", m_PackageData->GameConfig->velocityIterations)
			.AddKeyValuePair("gravity", m_PackageData->GameConfig->gravity)
			.EndTable() // PhysicsParams
			.EndTable() // GameConfig
			.FinishStream();

		return settingsConfigPath;
	}

	std::string Packager::CreateAssetDefsFile(const std::string& tempFilepath, const rapidjson::Value& assets)
	{
		std::unique_ptr<LuaSerializer> serializer{ nullptr };
		if (!fs::exists(tempFilepath))
		{
			F_ERROR("Failed to create settings config file. '{}' does not exist", tempFilepath);
			return {};
		}

		const std::string assetsDefPath = tempFilepath + PATH_SEPARATOR + "assetDefs.lua";

		auto optProjectFilePath = m_PackageData->ProjectInfo->GetProjectFilePath();
		F_ASSERT(optProjectFilePath && "Project file path must be set in project info");
		if (!optProjectFilePath)
		{
			F_ERROR("Failed to create asset defs file. Project file path was not set in project info");
			return {};
		}

		try
		{
			serializer = std::make_unique<LuaSerializer>(assetsDefPath);
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to create asset defs file '{}': {}", assetsDefPath, ex.what());
			return {};
		}

		serializer->StartNewTable("Assets");
		serializer->StartNewTable("textures");
		if (assets.HasMember("textures"))
		{
			const rapidjson::Value& textures = assets["textures"];

			if (!textures.IsArray())
			{
				F_ERROR("Failed to create asset defs file in project file '{}'. Expecting \"textures\" as an array", optProjectFilePath->string());
				return {};
			}

			for (const auto& jsonTexture : textures.GetArray())
			{
				// "assets/[asset_type]/[extra_folders]/file"
				std::string textureName{ jsonTexture["name"].GetString() };
				std::string texturePath{ jsonTexture["path"].GetString() };
				bool pixelArt = jsonTexture["isPixelArt"].GetBool();

				serializer->StartNewTable()
					.AddKeyValuePair("name", textureName, true, false, false, true)
					.AddKeyValuePair("path", texturePath, true, false, false, true)
					.AddKeyValuePair("isPixelArt", pixelArt ? "true" : "false")
					.EndTable(); // Current Texture
			}
		}
		serializer->EndTable(); // textures

		serializer->StartNewTable("soundfx");
		if (assets.HasMember("soundfx"))
		{
			const rapidjson::Value& soundfx = assets["soundfx"];

			if (!soundfx.IsArray())
			{
				F_ERROR("Failed to create asset defs file in project file '{}'. Expecting \"soundfx\" as an array", optProjectFilePath->string());
				return {};
			}

			for (const auto& jsonSoundfx : soundfx.GetArray())
			{
				// "assets/[asset_type]/[extra_folders]/file"
				std::string soundFXName{ jsonSoundfx["name"].GetString() };
				std::string soundFXPath{ jsonSoundfx["path"].GetString() };

				serializer->StartNewTable()
					.AddKeyValuePair("name", soundFXName, true, false, false, true)
					.AddKeyValuePair("path", soundFXPath, true, false, false, true)
					.EndTable(); // Current SoundFX
			}
		}
		serializer->EndTable(); // soundfx

		serializer->StartNewTable("music");
		if (assets.HasMember("music"))
		{
			const rapidjson::Value& music = assets["music"];

			if (!music.IsArray())
			{
				F_ERROR("Failed to create asset defs file in project file '{}'. Expecting \"music\" as an array", optProjectFilePath->string());
				return {};
			}

			for (const auto& jsonMusic : music.GetArray())
			{
				// "assets/[asset_type]/[extra_folders]/file"
				std::string musicName{ jsonMusic["name"].GetString() };
				std::string musicPath{ jsonMusic["path"].GetString() };

				serializer->StartNewTable()
					.AddKeyValuePair("name", musicName, true, false, false, true)
					.AddKeyValuePair("path", musicPath, true, false, false, true)
					.EndTable(); // Current music
			}
		}
		serializer->EndTable(); // music

		serializer->StartNewTable("fonts");
		if (assets.HasMember("fonts"))
		{
			const rapidjson::Value& fonts = assets["fonts"];

			if (!fonts.IsArray())
			{
				F_ERROR("Failed to create asset defs file in project file '{}'. Expecting \"fonts\" as an array", optProjectFilePath->string());
				return {};
			}

			for (const auto& jsonFonts : fonts.GetArray())
			{
				// "assets/[asset_type]/[extra_folders]/file"
				std::string fontName{ jsonFonts["name"].GetString() };
				std::string fontPath{ jsonFonts["path"].GetString() };

				serializer->StartNewTable()
					.AddKeyValuePair("name", fontName, true, false, false, true)
					.AddKeyValuePair("path", fontPath, true, false, false, true)
					.AddKeyValuePair("fontSize", jsonFonts["fontSize"].GetFloat())
					.EndTable(); // Current font
			}
		}
		serializer->EndTable(); // fonts

		serializer->EndTable(); // Assets
		serializer->FinishStream();

		serializer.reset(nullptr);

		std::fstream assetDefs;
		assetDefs.open(assetsDefPath, std::ios::app | std::ios::out);
		if (!assetDefs.is_open())
		{
			F_ERROR("Failed to append assets file and add function");
			return {};
		}

		assetDefs << R"(
		function LoadAssets(assets)
			for k, v in pairs(assets) do
				for i = 1, #v do
					if k == "textures" then
						if not AssetManager.add_texture(v[i].name, v[i].path, v[i].isPixelArt) then
							print("Failed to load texture '"..v[i].name.."'")
						end
					elseif k == "fonts" then
						if not AssetManager.add_font(v[i].name, v[i].path, v[i].fontSize) then
							print("Failed to load font '"..v[i].name.."'")
						end
					elseif k == "soundfx" then
						if not AssetManager.add_sound(v[i].name, v[i].path) then
							print("Failed to load soundfx '"..v[i].name.."'")
						end
					elseif k == "music" then
						if not AssetManager.add_music(v[i].name, v[i].path) then
							print("Failed to load music '"..v[i].name.."'")
						end
					end
				end
			end
		end

		LoadAssets(Assets)
		)";

		assetDefs.close();

		return assetsDefPath;
	}

	std::vector<std::string> Packager::CreateSceneFiles(const std::string& tempFilepath, const rapidjson::Value& scenes)
	{
		return std::vector<std::string>();
	}

	void Packager::CopyFilesToDestination()
	{
	}

	void Packager::CopyAssetsToDestination()
	{
	}

}
