#include "ContactListenerBindings.h"

#include "Core/ECS/MetaUtilities.h"
#include "Physics/UserData.h"
#include "Logger/Logger.h"

namespace Feather {
    
    void ContactListenerBinder::CreateLuaContactListener(sol::state& lua, entt::registry& registry)
    {
		auto& contactListener = registry.ctx().get<std::shared_ptr<ContactListener>>();
		if (!contactListener)
		{
			F_ERROR("Failed to create the contact listener lua bind - Contact listener is not in the registry!");
			return;
		}

		lua.new_usertype<ContactListener>(
			"ContactListener",
			sol::no_constructor,
			"getUserData", [&](sol::this_state s) { return GetUserData(*contactListener, s); }
		);
    }

    std::tuple<sol::object, sol::object> ContactListenerBinder::GetUserData(ContactListener& contactListener, sol::this_state s)
    {
		auto pUserDataA = contactListener.GetUserDataA();
		auto pUserDataB = contactListener.GetUserDataB();

		if (!pUserDataA || !pUserDataB)
			return std::make_tuple(sol::lua_nil_t{}, sol::lua_nil_t{});

		F_ASSERT(pUserDataA->type_id != 0 && pUserDataB->type_id != 0 && "UserData type_id must be set!");

		using namespace entt::literals;

		const auto maybe_any_a = InvokeMetaFunction(static_cast<entt::id_type>(pUserDataA->type_id), "get_user_data"_hs, *pUserDataA, s);
		const auto maybe_any_b = InvokeMetaFunction(static_cast<entt::id_type>(pUserDataB->type_id), "get_user_data"_hs, *pUserDataB, s);

		if (!maybe_any_a || !maybe_any_b)
			return std::make_tuple(sol::lua_nil_t{}, sol::lua_nil_t{});

		return std::make_tuple(
			maybe_any_a.cast<sol::reference>(),
			maybe_any_b.cast<sol::reference>()
		);
    }

}
