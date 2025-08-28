#pragma once

#include <rapidjson/document.h>

namespace Feather {

	enum class AssetType;
	class ThreadPool;
	class LuaSerializer;

	struct AssetPackagerParams
	{
		std::string AssetsPath{};
		std::string TempFilePath{};
		std::string DestinationPath{};
		std::string ProjectPath{};
	};

	struct AssetConversionData
	{
		std::string inAssetFile{};
		std::string assetName{};
		AssetType type;
		std::optional<float> optFontSize{ std::nullopt };
		std::optional<bool> optPixelArt{ std::nullopt };
	};

	class AssetPackager
	{
	public:
		AssetPackager(const AssetPackagerParams& params, std::shared_ptr<ThreadPool> threadPool);
		~AssetPackager();

		void PackageAssets(const rapidjson::Value& assets);

	private:
		void ConvertAssetToLuaTable(LuaSerializer& luaSerializer, const AssetConversionData& conversionData);

		void CreateLuaAssetFiles(const std::string& projectPath, const rapidjson::Value& assets);
		bool CompileLuaAssetFiles();
		bool CreateAssetsZip();

		struct AssetPackageStatus
		{
			std::string Error{};
			bool Success{ false };
		};

		AssetPackageStatus SerializeAssetsByType(
			const rapidjson::Value& assets,
			const std::filesystem::path& tempAssetsPath,
			const std::string& assetTypeName,
			const std::string& contentPath,
			AssetType assetType);

	private:
		AssetPackagerParams m_Params;
		std::shared_ptr<ThreadPool> m_ThreadPool;
	};

}
