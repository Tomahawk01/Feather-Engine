#pragma once

#include "Physics/UserData.h"

#include <sol/sol.hpp>

namespace Feather {

	struct UserDataBinder
	{
		static void CreateLuaUserData(sol::state& lua);

		template <typename DATA>
		static void register_user_meta_data();
	};

	template <typename DATA>
	auto create_user_data(const sol::table& data, sol::this_state s);

	template <typename DATA>
	auto set_user_data(UserData* userData, const sol::table& data, sol::this_state s);

	template <typename DATA>
	auto get_user_data(UserData& userData, sol::this_state s);

}

#include "UserDataBindings.inl"
