#include "Physics/PhysicsUtilities.h"
#include "Utils/FeatherUtilities.h"

#include <format>
#include <map>
#include <algorithm>

namespace Feather {

	static const std::map<std::string, RigidBodyType> StringToRigidBodyType{
		{ "STATIC", RigidBodyType::STATIC },
		{ "KINEMATIC", RigidBodyType::KINEMATIC },
		{ "DYNAMIC", RigidBodyType::DYNAMIC }
	};

	static const std::map<RigidBodyType, std::string> RigidBodyTypeToString{
		{ RigidBodyType::STATIC, "STATIC" },
		{ RigidBodyType::KINEMATIC, "KINEMATIC" },
		{ RigidBodyType::DYNAMIC, "DYNAMIC" }
	};

	static std::map<std::string, FilterCategory> StringToFilterCategory{
		{ "NO_CATEGORY", FilterCategory::NO_CATEGORY },
		{ "PLAYER", FilterCategory::PLAYER },
		{ "ENEMY", FilterCategory::ENEMY },
		{ "ITEM", FilterCategory::ITEM },
		{ "WALLS", FilterCategory::WALLS },
		{ "GROUND", FilterCategory::GROUND },
		{ "TRIGGER", FilterCategory::TRIGGER },
		{ "PROJECTILE", FilterCategory::PROJECTILE },
		{ "CATEGORY_7", FilterCategory::CATEGORY_7 },
		{ "CATEGORY_8", FilterCategory::CATEGORY_8 },
		{ "CATEGORY_9", FilterCategory::CATEGORY_9 },
		{ "CATEGORY_10", FilterCategory::CATEGORY_10 },
		{ "CATEGORY_11", FilterCategory::CATEGORY_11 },
		{ "CATEGORY_12", FilterCategory::CATEGORY_12 },
		{ "CATEGORY_13", FilterCategory::CATEGORY_13 },
		{ "CATEGORY_14", FilterCategory::CATEGORY_14 },
		{ "CATEGORY_15", FilterCategory::CATEGORY_15 }
	};

	static std::map<FilterCategory, std::string> FilterCategoryToString{
		{ FilterCategory::NO_CATEGORY, "NO_CATEGORY" },
		{ FilterCategory::PLAYER, "PLAYER" },
		{ FilterCategory::ENEMY, "ENEMY" },
		{ FilterCategory::ITEM, "ITEM" },
		{ FilterCategory::WALLS, "WALLS" },
		{ FilterCategory::GROUND, "GROUND" },
		{ FilterCategory::TRIGGER, "TRIGGER" },
		{ FilterCategory::PROJECTILE, "PROJECTILE" },
		{ FilterCategory::CATEGORY_7, "CATEGORY_7" },
		{ FilterCategory::CATEGORY_8, "CATEGORY_8" },
		{ FilterCategory::CATEGORY_9, "CATEGORY_9" },
		{ FilterCategory::CATEGORY_10, "CATEGORY_10" },
		{ FilterCategory::CATEGORY_11, "CATEGORY_11" },
		{ FilterCategory::CATEGORY_12, "CATEGORY_12" },
		{ FilterCategory::CATEGORY_13, "CATEGORY_13" },
		{ FilterCategory::CATEGORY_14, "CATEGORY_14" },
		{ FilterCategory::CATEGORY_15, "CATEGORY_15" }
	};

	std::string GetFilterCategoryString(FilterCategory category)
	{
		auto filterCatItr = FilterCategoryToString.find(category);
		if (filterCatItr == FilterCategoryToString.end())
			return {};

		return filterCatItr->second;
	}

	FilterCategory GetFilterCategoryByString(const std::string& category)
	{
		auto filterCatItr = StringToFilterCategory.find(category);
		if (filterCatItr == StringToFilterCategory.end())
			return FilterCategory::NO_CATEGORY;

		return filterCatItr->second;
	}

	bool AddCustomFilterCategoryType(const std::string& newFilterCategoryType, std::string& errorStr)
	{
		if (StringToFilterCategory.contains(newFilterCategoryType))
		{
			errorStr = std::format("Filter category '{}' already exists", newFilterCategoryType);
			return false;
		}

		// Check if there are any free User Categories left
		auto catItr = std::ranges::find_if(StringToFilterCategory, [](const auto& pair) { return pair.first.starts_with("CATEGORY_"); });

		if (catItr == StringToFilterCategory.end())
		{
			errorStr = "There are no new filter categories available. Please remove one to add a new";
			return false;
		}

		std::string filterCatToChange = catItr->first;
		FilterCategory filterCategory = catItr->second;

		// Now we want to get the key and change the node name
		if (!KeyChange(StringToFilterCategory, filterCatToChange, newFilterCategoryType))
		{
			errorStr = "Failed to add new filter category. Unable to change the key";
			return false;
		}

		// Change the value of the other map as well
		FilterCategoryToString[filterCategory] = newFilterCategoryType;

		return true;
	}

	const std::map<FilterCategory, std::string> GetFilterCategoryToStringMap()
	{
		auto filteredValues = FilterCategoryToString | std::views::filter([](auto&& pair) { return !pair.second.starts_with("CATEGORY_"); });

		std::map<FilterCategory, std::string> filteredMap{ filteredValues.begin(), filteredValues.end() };
		return filteredMap;
	}

	std::string GetRigidBodyTypeString(RigidBodyType rigidType)
	{
		auto rigidItr = RigidBodyTypeToString.find(rigidType);
		if (rigidItr == RigidBodyTypeToString.end())
			return {};

		return rigidItr->second;
	}

	RigidBodyType GetRigidBodyTypeByString(const std::string rigidType)
	{
		auto rigidItr = StringToRigidBodyType.find(rigidType);
		if (rigidItr == StringToRigidBodyType.end())
			return RigidBodyType::STATIC;

		return rigidItr->second;
	}

	const std::map<RigidBodyType, std::string>& GetRigidBodyStringMap()
	{
		return RigidBodyTypeToString;
	}

}
