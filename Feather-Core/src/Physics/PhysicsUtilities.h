#pragma once

#include <cstdint>

namespace Feather {

	enum class RigidBodyType
	{
		STATIC = 0,		// Zero mass, zero velocity, may be manually moved
		KINEMATIC,		// Zero mass, velocity set by user, moved by solver
		DYNAMIC			// Positive mass, velocity determined by forces, moved by solver
	};

	enum class FilterCategory : std::uint16_t
	{
		// Default Types
		NO_CATEGORY = 0,
		PLAYER = 1 << 0,
		ENEMY = 1 << 1,
		ITEM = 1 << 2,
		WALLS = 1 << 3,
		GROUND = 1 << 4,
		TRIGGER = 1 << 5,
		PROJECTILE = 1 << 6,

		// User defined object types
		// NOTE: These can have their visual name changed in the editor
		CATEGORY_7 = 1 << 7,
		CATEGORY_8 = 1 << 8,
		CATEGORY_9 = 1 << 9,
		CATEGORY_10 = 1 << 10,
		CATEGORY_11 = 1 << 11,
		CATEGORY_12 = 1 << 12,
		CATEGORY_13 = 1 << 13,
		CATEGORY_14 = 1 << 14,
		CATEGORY_15 = 1 << 15
	};

	/*
	* @brief Get the string value of the filter category type
	* @param Takes in an FilterCategory enum
	* @return Returns the string representation of the FilterCategory passed in
	*/
	std::string GetFilterCategoryString(FilterCategory category);

	/*
	* @brief Get the FilterCategory based on the passed in string.
	* The actual filter category may differ from the visual name in the editor
	* @param The string value of the desired FilterCategory
	* @return Returns the FilterCategory if successful, NO_CATEGORY is returned otherwise
	*/
	FilterCategory GetFilterCategoryByString(const std::string& category);

	/*
	* @brief Checks to see if there are any free user categories that could be added to the FilterCategories
	* @param Const string ref for new filter category name
	* @param String ref for the error
	* @return Returns true if successful, otherwise returns false with a populated error string
	*/
	bool AddCustomFilterCategoryType(const std::string& newFilterCategoryType, std::string& errorStr);

	/*
	* @brief Returns a map for the filter catergories and their visual representation
	*/
	const std::map<FilterCategory, std::string> GetFilterCategoryToStringMap();

	std::vector<std::string> GetFilterStrings();

	std::string GetRigidBodyTypeString(RigidBodyType rigidType);
	RigidBodyType GetRigidBodyTypeByString(const std::string rigidType);
	const std::map<RigidBodyType, std::string>& GetRigidBodyStringMap();

}
