#pragma once

#include <sol/sol.hpp>
#include <entt.hpp>

namespace Feather {

	class ContactListener;

	class ContactListenerBinder
	{
	public:
		static void CreateLuaContactListener(sol::state& lua, entt::registry& registry);

	private:
		static std::tuple<sol::object, sol::object> GetUserData(ContactListener& contactListener, sol::this_state s);
	};

}
