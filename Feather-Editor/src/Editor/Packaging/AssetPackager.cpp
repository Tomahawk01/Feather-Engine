#include "AssetPackager.h"

#include "Logger/Logger.h"
#include "Utils/FeatherUtilities.h"
#include "Utils/HelperUtilities.h"
#include "Utils/ThreadPool.h"
#include "FileSystem/Serializers/LuaSerializer.h"
#include "ScriptCompiler.h"

#include <libzippp/libzippp.h>

#include <format>
#include <vector>

namespace fs = std::filesystem;

namespace Feather {

	AssetPackager::AssetPackager(const AssetPackagerParams& params, std::shared_ptr<ThreadPool> threadPool)
		: m_Params{ params }
		, m_ThreadPool{ threadPool }
	{}

	AssetPackager::~AssetPackager() = default;

	void AssetPackager::PackageAssets(const rapidjson::Value& assets)
	{
		try
		{
			CreateLuaAssetFiles(m_Params.ProjectPath, assets);

			if (!CompileLuaAssetFiles())
			{
				F_ERROR("Failed to compile assets");
				return;
			}

			if (!CreateAssetsZip())
			{
				F_ERROR("Failed to archive assets");
				return;
			}
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to package assets: {}", ex.what());
		}
	}

	void AssetPackager::ConvertAssetToLuaTable(LuaSerializer& luaSerializer, const std::string& assetNum, const std::string& inAssetFile, AssetType type)
	{
		std::fstream in{ inAssetFile, std::ios::in | std::ios::binary };
		if (!in.is_open())
			throw std::runtime_error(std::format("Failed to open file '{}'", inAssetFile));

		fs::path assetPath{ inAssetFile };

		int readByte{ 0 };
		std::size_t i{ 0U };
		std::size_t count{ 0U };

		try
		{
			luaSerializer.StartNewTable(assetNum)
				.AddKeyValuePair("assetName", assetPath.stem().string(), true, false, false, true)
				.AddKeyValuePair("assetExt", assetPath.extension().string(), true, false, false, true)
				.AddKeyValuePair("assetType", AssetTypeToString(type), true, false, false, true);

			luaSerializer.StartNewTable("data");

			while ((readByte = in.get()) != EOF)
			{
				if (count >= 100)
				{
					luaSerializer.AddWords(",", true);
					count = 0;
				}

				luaSerializer.AddValue(std::format("{:#04x}", readByte), false);
				++count;
				++i;
			}

			luaSerializer
				.EndTable()
				.AddKeyValuePair("dataEnd", i - 1ull)
				.AddKeyValuePair("dataSize", i)
				.EndTable();
		}
		catch (const std::exception& ex)
		{
			throw std::runtime_error(std::format("Failed to write '{}' at path '{}' to asset file: {}", assetNum, inAssetFile, ex.what()));
		}
	}

	void AssetPackager::CreateLuaAssetFiles(const std::string& projectPath, const rapidjson::Value& assets)
	{
		if (!fs::exists(fs::path{ m_Params.TempFilePath }))
		{
			throw std::runtime_error(std::format("Failed to create lua asset files. Temp path '{}' does not exist or is invalid", m_Params.TempFilePath));
		}

		fs::path tempAssetPath{ std::format("{}{}{}", m_Params.TempFilePath, PATH_SEPARATOR, "assets")};
		m_Params.AssetsPath = tempAssetPath.string();

		if (!fs::exists(tempAssetPath))
		{
			fs::create_directories(tempAssetPath);
		}

		std::string contentPath = projectPath + PATH_SEPARATOR + "content";
		if (!fs::exists(fs::path{ contentPath }))
		{
			throw std::runtime_error(std::format("Failed to create lua asset files. Content path '{}' does not exist or is invalid", contentPath));
		}

		std::vector<std::future<AssetPackageStatus>> assetFutures;
		assetFutures.emplace_back(
			m_ThreadPool->Enqueue(
				[&] { return SerializeAssetsByType(assets, tempAssetPath, "textures", contentPath, AssetType::TEXTURE); }
		));

		assetFutures.emplace_back(
			m_ThreadPool->Enqueue(
				[&] { return SerializeAssetsByType(assets, tempAssetPath, "soundfx", contentPath, AssetType::SOUNDFX); }
		));

		assetFutures.emplace_back(
			m_ThreadPool->Enqueue(
				[&] { return SerializeAssetsByType(assets, tempAssetPath, "music", contentPath, AssetType::MUSIC); }
		));

		assetFutures.emplace_back(
			m_ThreadPool->Enqueue(
				[&] { return SerializeAssetsByType(assets, tempAssetPath, "fonts", contentPath, AssetType::FONT); }
		));

		bool hasError{ false };
		std::string errorStr{};

		std::ranges::for_each(
			assetFutures, [&](auto& fut)
			{
				try
				{
					auto status = fut.get();
					if (!status.Success)
					{
						hasError = true;
						errorStr += status.Error + "\n";
					}
				}
				catch (...)
				{
					hasError = true;
					errorStr += "Failed to serialize assets. Unknown error.\n";
				}
			}
		);

		if (hasError)
		{
			throw std::runtime_error(std::format("Failed to serialize assets correctly: {}", errorStr));
		}
	}

	bool AssetPackager::CompileLuaAssetFiles()
	{
		ScriptCompiler scriptCompiler{};

		for (const auto& entry : fs::directory_iterator(fs::path{ m_Params.AssetsPath }))
		{
			if (fs::is_directory(entry))
				continue;

			if (fs::is_regular_file(entry.path()) && entry.path().extension() == ".fasset")
			{
				if (!scriptCompiler.AddScript(entry.path().string()))
				{
					F_ERROR("Failed to add script: '{}' to asset packager", entry.path().string());
					return false;
				}

				scriptCompiler.SetOutputFileName(std::string{ (fs::path{ m_Params.TempFilePath } / std::string{ entry.path().stem().string() + ".luac" }).string() });

				scriptCompiler.Compile();
				scriptCompiler.ClearScripts();
			}
		}

		return true;
	}

	bool AssetPackager::CreateAssetsZip()
	{
		fs::path assetsDestination{ m_Params.DestinationPath };
		if (!fs::exists(assetsDestination))
		{
			std::error_code ec;
			if (!fs::create_directories(assetsDestination, ec))
			{
				F_ERROR("Failed to create directory '{}'", assetsDestination.string());
				return false;
			}
		}

		assetsDestination /= "FeatherAssets.zip";
		libzippp::ZipArchive zip{ assetsDestination.string() };

		zip.setErrorHandlerCallback(
			[](const std::string& message, const std::string& strError, int zipErrorCode, int systemErrorCode)
			{
				F_ERROR("Failed to archive assets: {}\nError: {}", message, strError);
			});

		if (!zip.open(libzippp::ZipArchive::Write))
		{
			F_ERROR("Failed to open zip: {}", zip.getPath());
			return false;
		}

		if (!zip.addEntry(std::string{"FeatherAssets/"}))
		{
			F_ERROR("Failed to add entry to archive");
			zip.close();
			return false;
		}

		const std::string texturePath{ std::format("{}{}{}", m_Params.TempFilePath, PATH_SEPARATOR, "textures.luac")};
		if (fs::exists(texturePath))
		{
			if (!zip.addFile(std::format("{}{}{}", "FeatherAssets", PATH_SEPARATOR, "textures.luac"), texturePath))
			{
				F_ERROR("Failed to add textures.luac to zip");
				zip.close();
				return false;
			}
		}

		const std::string musicPath{ std::format("{}{}{}", m_Params.TempFilePath, PATH_SEPARATOR, "music.luac") };
		if (fs::exists(musicPath))
		{
			if (!zip.addFile(std::format("{}{}{}", "FeatherAssets", PATH_SEPARATOR, "music.luac"), musicPath))
			{
				F_ERROR("Failed to add music.luac to zip");
				zip.close();
				return false;
			}
		}

		const std::string soundFxPath{ std::format("{}{}{}", m_Params.TempFilePath, PATH_SEPARATOR, "soundfx.luac") };
		if (fs::exists(soundFxPath))
		{
			if (!zip.addFile(std::format("{}{}{}", "FeatherAssets", PATH_SEPARATOR, "soundfx.luac"), soundFxPath))
			{
				F_ERROR("Failed to add soundfx.luac to zip");
				zip.close();
				return false;
			}
		}

		const std::string fontPath{ std::format("{}{}{}", m_Params.TempFilePath, PATH_SEPARATOR, "fonts.luac") };
		if (fs::exists(fontPath))
		{
			if (!zip.addFile(std::format("{}{}{}", "FeatherAssets", PATH_SEPARATOR, "fonts.luac"), fontPath))
			{
				F_ERROR("Failed to add fonts.luac to zip");
				zip.close();
				return false;
			}
		}

		zip.close();
		return true;
	}

	AssetPackager::AssetPackageStatus AssetPackager::SerializeAssetsByType(
		const rapidjson::Value& assets,
		const std::filesystem::path& tempAssetsPath,
		const std::string& assetTypeName,
		const std::string& contentPath,
		AssetType assetType)
	{
		const std::string assetFile{ assetTypeName + ".fasset" };
		fs::path assetPath = tempAssetsPath / assetFile;

		std::unique_ptr<LuaSerializer> luaSerializer{ nullptr };
		try
		{
			luaSerializer = std::make_unique<LuaSerializer>(assetPath.string());
		}
		catch (const std::exception& ex)
		{
			std::string error{ std::format("Failed to serialize assets: {}", ex.what()) };
			return { .Error = error, .Success = false };
		}

		if (assets.HasMember(assetTypeName.c_str()))
		{
			const rapidjson::Value& assetArray = assets[assetTypeName.c_str()];
			if (!assetArray.IsArray())
			{
				std::string error{ std::format("Failed to serialize asset file '{}'. Expecting '{}' must be an array", tempAssetsPath.string(), assetTypeName) };
				return { .Error = error, .Success = false };
			}

			luaSerializer->StartNewTable("F_Assets");

			try
			{
				for (const auto& jsonValue : assetArray.GetArray())
				{
					std::string path{ contentPath + PATH_SEPARATOR + jsonValue["path"].GetString() };
					ConvertAssetToLuaTable(*luaSerializer, {}, path, assetType);
				}
			}
			catch (const std::exception& ex)
			{
				std::string error{ std::format("Failed to convert asset to lua table: {}", ex.what()) };
				return { .Error = error, .Success = false };
			}

			luaSerializer->EndTable();
		}

		return { .Success = true };
	}

}
