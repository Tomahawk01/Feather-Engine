#pragma once

namespace Feather {

	enum class AssetType;

	class AssetDisplayUtils
	{
	public:
		AssetDisplayUtils() = delete;
		static std::vector<const char*> GetAssetFileFilters(AssetType assetType);
		static const char* GetAssetDescriptionByType(AssetType assetType);
		static std::string AddAssetBasedOnType(AssetType assetType);
		static void OpenAddAssetModalBasedOnType(AssetType assetType, bool* isOpen);
	};

}
