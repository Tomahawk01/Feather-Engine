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

#ifdef DEBUG
constexpr std::array<std::string_view, 16> CopyPackageFiles = {
	"lua.dll",
	//"ogg.dll",
	"Feather-CrashReporter.exe",
	"Feather-Runtime.exe",
	"SDL2_image.dll",
	"SDL2_mixer.dll",
	"SDL2.dll",
	//"vorbis.dll",
	//"vorbisfile.dll",
	//"wavpackdll.dll",
	"zlibd1.dll",
	"bz2d.dll",
	"zip.dll",
	"libzippp.dll",
	//"mpg123.dll"
};
#else
constexpr std::array<std::string_view, 16> CopyPackageFiles = {		// TODO: Test this !!!
	"lua.dll",
	//"ogg.dll",
	"Feather-CrashReporter.exe",
	"Feather-Runtime.exe",
	"SDL2_image.dll",
	"SDL2_mixer.dll",
	"SDL2.dll",
	//"vorbis.dll",
	//"vorbisfile.dll",
	//"wavpackdll.dll",
	"zlib.dll",
	"bz2.dll",
	"zip.dll",
	"libzippp.dll",
	//"mpg123.dll"
};
#endif

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
		if (!m_PackageData)
		{
			F_ERROR("Failed to run packager");
			return;
		}

		m_Packaging = true;

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
			F_ERROR("Failed to delete packaging temp data at path '{}': {}", m_PackageData->TempDataPath, err.what());
			m_Packaging = false;
			return;
		}

		try
		{
			UpdateProgress(0.0f, "Starting packaging");
			if (fs::exists(fs::path{ m_PackageData->FinalDestination }))
			{
				F_ERROR("Failed to package game. '{}' directory already exists", m_PackageData->FinalDestination);
				UpdateProgress(0.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			UpdateProgress(5.0f, "Creating package destination");
			std::error_code ec;
			if (!fs::create_directory(fs::path{ m_PackageData->FinalDestination }, ec))
			{
				F_ERROR("Failed to create directory '{}': {}", m_PackageData->FinalDestination, ec.message());
				UpdateProgress(5.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			std::this_thread::sleep_for(1s);

			UpdateProgress(10.0f, "Creating temp data");
			if (!fs::exists(fs::path{ m_PackageData->TempDataPath }))
			{
				if (!fs::create_directory(m_PackageData->TempDataPath))
				{
					F_ERROR("Failed to create temporary data path at '{}'", m_PackageData->TempDataPath);
					UpdateProgress(10.0f, "Packaging failed. Please see logs");
					m_HasError = true;
					return;
				}
			}

			std::this_thread::sleep_for(1s);

			UpdateProgress(25.0f, "Adding game lua scripts");
			auto pScriptCompiler = std::make_unique<ScriptCompiler>();
			auto optScriptListPath = m_PackageData->ProjectInfo->GetScriptListPath();
			F_ASSERT(optScriptListPath && "Script List Path must be set");
			if (!optScriptListPath)
			{
				F_ERROR("Failed to add scripts. Script list path was not set in the project info");
				UpdateProgress(25.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			if (!pScriptCompiler->AddScripts(optScriptListPath->string()))
			{
				F_ERROR("Failed to add scripts");
				UpdateProgress(25.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			UpdateProgress(50.0f, "Start asset copying");

			auto optProjectFilePath = m_PackageData->ProjectInfo->GetProjectFilePath();
			F_ASSERT(optProjectFilePath && "Project File path must be set in project info");
			if (!optProjectFilePath)
			{
				F_ERROR("Failed to open project file. Project file path was not set in the project info");
				UpdateProgress(50.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			// NOTE: Read in the files from the json project file
			std::ifstream procFile;
			procFile.open(optProjectFilePath->string());

			if (!procFile.is_open())
			{
				F_ERROR("Failed to open project file '{}'", optProjectFilePath->string());
				UpdateProgress(50.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			std::stringstream ss;
			ss << procFile.rdbuf();
			std::string contents = ss.str();
			rapidjson::StringStream jsonStr{ contents.c_str() };

			rapidjson::Document doc;
			doc.ParseStream(jsonStr);

			if (doc.HasParseError() || !doc.IsObject())
			{
				F_ERROR("Failed to load Project: file '{}' is not valid JSON, Error: {}, Offset: {}", optProjectFilePath->string(), rapidjson::GetParseError_En(doc.GetParseError()), doc.GetErrorOffset());
				UpdateProgress(50.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			// NOTE: Get the project data
			if (!doc.HasMember("project_data"))
			{
				F_ERROR("Failed to load project: file '{}'. Expecting \"project_data\" member in project file", optProjectFilePath->string());
				UpdateProgress(50.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			const rapidjson::Value& projectData = doc["project_data"];

			// NOTE: Load all assets
			if (!projectData.HasMember("assets"))
			{
				F_ERROR("Failed to load project: file '{}'. Expecting \"assets\" member in project file", optProjectFilePath->string());
				UpdateProgress(50.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			const rapidjson::Value& assets = projectData["assets"];

			if (m_PackageData->GameConfig->packageAssets)
			{
				UpdateProgress(60.0f, "Starting packaging of assets");
				AssetPackagerParams assetPackagerParams{
					.TempFilePath = m_PackageData->TempDataPath,
					.DestinationPath = m_PackageData->FinalDestination + PATH_SEPARATOR + "assets",
					.ProjectPath = m_PackageData->ProjectInfo->GetProjectPath().string() };

				AssetPackager assetPackager{ assetPackagerParams, m_ThreadPool };

				assetPackager.PackageAssets(assets);
			}
			else
			{
				UpdateProgress(60.0f, "Creating asset defs luac file");
				std::string sAssetDefsFile = CreateAssetDefsFile(m_PackageData->TempDataPath, assets);
				if (!fs::exists(sAssetDefsFile))
				{
					F_ERROR("Failed to create asset defs file");
					UpdateProgress(60.0f, "Packaging failed. Please see logs");
					m_HasError = true;
					return;
				}

				UpdateProgress(65.0f, "Adding asset defs file to script compiler");
				if (!pScriptCompiler->AddScript(sAssetDefsFile))
				{
					F_ERROR("Failed to find the assetDefs.lua file at path '{}'", sAssetDefsFile);
					UpdateProgress(65.0f, "Packaging failed. Please see logs");
					m_HasError = true;
					return;
				}
			}

			UpdateProgress(70.0f, "Start packaging of all scenes");
			// We need to load all the scenes
			if (!assets.HasMember("scenes"))
			{
				F_ERROR("Failed to load project: file '{}'. Expecting \"scenes\" member in project file", optProjectFilePath->string());

				UpdateProgress(70.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			UpdateProgress(75.0f, "Creating scene Files");
			const rapidjson::Value& scenes = assets["scenes"];
			auto sceneFiles = CreateSceneFiles(m_PackageData->TempDataPath, scenes);

			if (sceneFiles.empty())
			{
				F_ERROR("Failed to create scene files or scene files are invalid");
				UpdateProgress(75.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			for (const auto& sSceneFile : sceneFiles)
			{
				if (!pScriptCompiler->AddScript(sSceneFile))
				{
					F_ERROR("Failed to add scene file [{}] to scripts to be compiled", sSceneFile);
					UpdateProgress(75.0f, "Packaging failed. Please see logs");
					m_HasError = true;
					return;
				}
			}

			UpdateProgress(80.0f, "Adding main lua script");

			auto optMainLuaScript = m_PackageData->ProjectInfo->GetMainLuaScriptPath();
			F_ASSERT(optMainLuaScript && "Main Lua script not set in the project info");
			if (!optMainLuaScript)
			{
				F_ERROR("Failed to add main.lua script. Path not set in the project info");
				UpdateProgress(80.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			if (!fs::exists(*optMainLuaScript))
			{
				F_ERROR("Failed to find the main.lua file at path '{}'", optMainLuaScript->string());
				UpdateProgress(80.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			if (!pScriptCompiler->AddScript(optMainLuaScript->string()))
			{
				F_ERROR("Failed to add main.lua script");
				m_HasError = true;
				return;
			}

			pScriptCompiler->SetOutputFileName(std::format("{}{}master.luac", m_PackageData->TempDataPath, PATH_SEPARATOR));

			UpdateProgress(85.0f, "Compiling game lua scripts");
			pScriptCompiler->Compile();

			UpdateProgress(87.0f, "Creating config.lua file");
			std::string sConfigFile = CreateConfigFile(m_PackageData->TempDataPath);

			if (!fs::exists(sConfigFile))
			{
				F_ERROR("Failed to create config file while packaging");
				UpdateProgress(87.0f, "Packaging failed. Please see logs");
				m_HasError = true;
				return;
			}

			pScriptCompiler->ClearScripts();
			pScriptCompiler->AddScript(sConfigFile);
			pScriptCompiler->SetOutputFileName(std::format("{}{}config.luac", m_PackageData->TempDataPath, PATH_SEPARATOR));
			UpdateProgress(90.0f, "Compiling config file");
			pScriptCompiler->Compile();
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to package game: {}", ex.what());
			UpdateProgress(0.0f, "Packaging failed. Please see logs");
			m_HasError = true;
			return;
		}

		UpdateProgress(95.0f, "Copying necessary files to packaged game destination");
		CopyFilesToDestination();

		UpdateProgress(100.0f, "Packaging Complete");
		m_Packaging = false;
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
		std::vector<std::string> sceneFiles;
		if (!scenes.IsArray())
		{
			F_ERROR("Failed to create scene lua files. \"scenes\" was not a valid json array");
			return sceneFiles;
		}

		auto optContentPath = m_PackageData->ProjectInfo->TryGetFolderPath(EProjectFolderType::Content);
		F_ASSERT(optContentPath && "Content path was not set in project info");
		if (!optContentPath)
		{
			F_ERROR("Failed to create scene lua files. Content path not set in project info");
			return sceneFiles;
		}

		for (const auto& jsonScene : scenes.GetArray())
		{
			std::string sceneName{ jsonScene["name"].GetString() };
			std::string sceneData{ jsonScene["sceneData"].GetString() };
			fs::path sceneDataPath = *optContentPath / sceneData;

			Registry registry;
			auto pSceneObject = std::make_unique<SceneObject>(sceneName, sceneDataPath.string());
			auto [tilemap, objectMap] = pSceneObject->ExportSceneToLua(sceneName, m_PackageData->TempDataPath, registry);

			if (!fs::exists(fs::path{ tilemap }) || !fs::exists(fs::path{ objectMap }))
			{
				F_ERROR("Failed to create scene files for scene '{}'", sceneName);
				return {};
			}

			sceneFiles.push_back(tilemap);
			sceneFiles.push_back(objectMap);
		}

		return sceneFiles;
	}

	void Packager::CopyFilesToDestination()
	{
	}

	void Packager::CopyAssetsToDestination()
	{
	}

}
