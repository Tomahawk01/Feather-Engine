#include "FeatherUtilities.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace Feather {

	std::string AssetTypeToString(AssetType assetType)
	{
		switch (assetType)
		{
			case AssetType::TEXTURE: return "TEXTURE";
			case AssetType::FONT: return "FONT";
			case AssetType::SOUNDFX: return "SOUNDFX";
			case AssetType::MUSIC: return "MUSIC";
			case AssetType::SCENE: return "SCENE";
			case AssetType::SHADER: return "SHADER";
			case AssetType::PREFAB: return "PREFAB";
			case AssetType::NO_TYPE: return "NO_TYPE";
		}

		return { "NO_TYPE" };
	}

	AssetType StringToAssetType(const std::string& assetType)
	{
		return AssetType();
	}

	std::string GetSubstring(std::string_view str, std::string_view find)
	{
		if (find.empty())
			return {};

		if (find.size() > 1)
		{
			auto const found = str.find(find);
			if (found == std::string_view::npos)
				return {};

			return std::string{ str.substr(found) };
		}

		auto const found = str.find_last_of(find[0]);
		if (found == std::string_view::npos || found + 1 >= str.size())
			return {};

		return std::string{ str.substr(found + 1) };
	}

	std::string ConvertWideToANSI(const std::wstring& wstr)
	{
#ifdef _WIN32
		int count = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
		std::string str(count, 0);
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
		return str;
#else
		return {};
#endif
	}

	std::wstring ConvertAnsiToWide(const std::string& str)
	{
#ifdef _WIN32
		int count = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), NULL, 0);
		std::wstring wstr(count, 0);
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), &wstr[0], count);
		return wstr;
#else
		return {};
#endif
	}

	std::string ConvertWideToUtf8(const std::wstring& wstr)
	{
#ifdef _WIN32
		int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
		std::string str(count, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
		return str;
#else
		return {};
#endif
	}

	std::wstring ConvertUtf8ToWide(const std::string& str)
	{
#ifdef _WIN32
		int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
		std::wstring wstr(count, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], count);
		return wstr;
#else
		return {};
#endif
	}

}
