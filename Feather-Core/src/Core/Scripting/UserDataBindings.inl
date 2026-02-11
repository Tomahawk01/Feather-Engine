#include "UserDataBindings.h"

#include "Physics/UserData.h"

#include <entt.hpp>

namespace Feather {

	template <typename DATA>
	inline auto create_user_data(const sol::table& data, sol::this_state s)
	{
		auto newData = data.valid() ? data.as<DATA>() : DATA{};

		UserData userData{};
		userData.userData = newData;
		userData.type_id = entt::type_hash<DATA>::value();

		return sol::make_reference(s, userData);
	}

	template <typename DATA>
	inline auto set_user_data(UserData* userData, const sol::table& data, sol::this_state s)
	{
		auto newData = data.valid() ? data.as<DATA>() : DATA{};

		userData->userData.reset();
		userData->userData = newData;
		userData->type_id = entt::type_hash<DATA>::value();

		return sol::make_reference(s, std::any_cast<DATA>(userData->userData));
	}

	template <typename DATA>
	inline auto get_user_data(UserData& userData, sol::this_state s)
	{
		auto& data = userData.userData;
		return sol::make_reference(s, std::any_cast<DATA>(data));
	}

	template <typename DATA>
	inline void UserDataBinder::register_user_meta_data()
	{
		using namespace entt::literals;
		entt::meta<DATA>()
			.type(entt::type_hash<DATA>::value())
			.template func<&create_user_data<DATA>>("create_user_data"_hs)
			.template func<&set_user_data<DATA>>("set_user_data"_hs)
			.template func<&get_user_data<DATA>>("get_user_data"_hs);
	}

}
