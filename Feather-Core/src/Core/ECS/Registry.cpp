#include "Registry.h"

#include "Entity.h"
#include "MetaUtilities.h"
#include "ECSUtils.h"

namespace Feather {

	Registry::Registry()
		: m_Registry{ nullptr }
	{
		m_Registry = std::make_unique<entt::registry>();
	}

	void Registry::CreateLuaRegistryBind(sol::state& lua, Registry& registry)
	{
		using namespace entt::literals;

		lua.new_usertype<entt::runtime_view>(
			"runtime_view",
			sol::no_constructor,
			"for_each",
			[&](const entt::runtime_view& view, const sol::function& callback, sol::this_state s)
			{
				if (!callback.valid())
					return;

				for (auto entity : view)
				{
					Entity ent{ registry, entity };
					callback(ent);
				}
			},
			"exclude",
			[&](entt::runtime_view& view, const sol::variadic_args& va)
			{
				for (const auto& type : va)
				{
					if (!type.as<sol::table>().valid())
						continue;

					const auto excluded_view = InvokeMetaFunction(
						GetIdType(type),
						"exclude_component_from_view"_hs,
						&registry,
						view
					);

					view = excluded_view ? excluded_view.cast<entt::runtime_view>() : view;
				}
			}
		);

		lua.new_usertype<Registry>(
			"Registry",
			sol::no_constructor,
			"get_entities",
			[&](const sol::variadic_args& va)
			{
				entt::runtime_view view{};
				for (const auto& type : va)
				{
					if (!type.as<sol::table>().valid())
						continue;

					const auto entities = InvokeMetaFunction(
						GetIdType(type),
						"add_component_to_view"_hs,
						&registry,
						view
					);

					view = entities ? entities.cast<entt::runtime_view>() : view;
				}

				return view;
			},
			"findEntityByTag",
			[&](const std::string& tag, sol::this_state s)
			{
				auto entity = FindEntityByTag(registry, tag);

				return entity == entt::null ? sol::lua_nil_t{} : sol::make_reference(s, Entity{ registry, entity });
			},
			"clear", [&]() { registry.GetRegistry().clear(); }
		);
	}

}
