#pragma once

#include <string>
#include <vector>

namespace Feather {

	enum class AssetType;

	class AssetDisplayUtils
	{
	public:
		AssetDisplayUtils() = delete;
		static std::vector<const char*> GetAssetFileFilters(AssetType assetType);
		static std::string AddAssetBasedOnType(AssetType assetType);
		static void OpenAddAssetModalBasedOnType(AssetType assetType, bool* isOpen);
	};

}
