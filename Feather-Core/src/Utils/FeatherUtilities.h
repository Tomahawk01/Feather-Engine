#pragma once

#include <concepts>
#include <string_view>

namespace Feather {

	enum class AssetType
	{
		TEXTURE = 0,
		FONT,
		SOUNDFX,
		MUSIC,
		SCENE,
		SHADER,
		PREFAB,

		NO_TYPE
	};

	/* @brief Helper struct used for loading zipped or archived assets.
	* Archived assets are the games assets that have been converted into luac files.
	*/
	struct FAsset
	{
		/* The name of the asset */
		std::string name{ };
		/* The size of the asset data. Based on a char array */
		size_t assetSize{ 0 };
		/* The end position of the asset data */
		size_t assetEnd{ 0 };
		/* The type of asset that the data represents */
		AssetType type{ AssetType::NO_TYPE };
		/* The underlying hex data of the asset */
		std::vector<unsigned char> assetData;
		/* Optional parameter if asset is font */
		std::optional<float> optFontSize{ std::nullopt };
		/* Optional parameter if asset is a texture */
		std::optional<bool> optPixelArt{ std::nullopt };
	};

	/* Ensure the types that are passed in are associative map types */
	template <typename T>
	concept MapType = std::same_as<T, std::map<typename T::key_type, typename T::mapped_type, typename T::key_compare, typename T::allocator_type>> ||
		std::same_as<T, std::unordered_map<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal, typename T::allocator_type>>;

	template <MapType Map>
	std::vector<typename Map::key_type> GetKeys(const Map& map)
	{
		auto keyView = std::views::keys(map);
		std::vector<typename Map::key_type> keys{ keyView.begin(), keyView.end() };
		return keys;
	}

	template <MapType Map, typename Func>
	std::vector<typename Map::key_type> GetKeys(const Map& map, Func func)
	{
		auto keyView = map | std::views::filter(func) | std::views::keys;
		std::vector<typename Map::key_type> keys{ keyView.begin(), keyView.end() };
		return keys;
	}

	template <MapType Map>
	bool KeyChange(Map& map, const typename Map::key_type& key, const typename Map::key_type& change)
	{
		if (!map.contains(key) || map.contains(change))
			return false;

		auto node = map.extract(key);
		node.key() = change;
		const auto [itr, isSuccess, nType] = map.insert(std::move(node));
		return isSuccess;
	}

	template <typename TCont, typename TFunc>
	bool CheckContainsValue(TCont& cont, TFunc func)
	{
		auto itr = std::ranges::find_if(cont, func);
		return itr == cont.end();
	}

	template <typename T>
	concept UintType = requires(T param) { requires std::is_integral_v<T>&& std::is_unsigned_v<T>; };

	template <UintType T>
	T SetBit(T num, int pos)
	{
		assert(pos < std::numeric_limits<T>::digits && "pos must be less than the number of digits");
		T mask = 1 << pos;
		return num | mask;
	}

	/* @brief Returns the first set bit in the given number */
	template <UintType T>
	T GetFirstSetBit(T num)
	{
		int position{ 1 }, i{ 1 };
		while (!(i & num))
		{
			i <<= 1;
			++position;
		}

		return position;
	}

	template <UintType T>
	T Bit(const T& x)
	{
		return T(1) << x;
	}

	template <UintType T>
	std::set<T> GetAllSetBits(T num)
	{
		std::set<T> setBits{};
		int position{ 0 };
		while (num)
		{
			if (num & 1)
				setBits.insert(position);

			++position;
			num >>= 1;
		}

		return setBits;
	}

	template <UintType T>
	void ResetBit(T& num, int bit)
	{
		num &= ~(1 << bit);
	}

	template <UintType T>
	bool IsBitSet(const T& x, const T& y)
	{
		return 0 != (x & y);
	}

	std::string AssetTypeToString(AssetType assetType);
	AssetType StringToAssetType(const std::string& assetType);

	std::string GetSubstring(std::string_view str, std::string_view find);

	std::string ConvertWideToANSI(const std::wstring& wstr);
	std::wstring ConvertAnsiToWide(const std::string& str);
	std::string ConvertWideToUtf8(const std::wstring& wstr);
	std::wstring ConvertUtf8ToWide(const std::string& str);

}

#define GET_SUBSTRING(str, find) Feather::GetSubstring(str, find)
