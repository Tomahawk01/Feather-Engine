#pragma once

#include <entt.hpp>
#include <sol/sol.hpp>

#include "Logger/Logger.h"

namespace Feather {

	[[nodiscard]] entt::id_type GetIdType(const sol::table& comp);

	template <typename ...Args>
	inline auto InvokeMetaFunction(entt::meta_type meta, entt::id_type func_id, Args&& ...args)
	{
		if (!meta)
		{
			F_ERROR("No entt::meta_type has been provided or is invalid!");
			assert(false && "No entt::meta_type has been provided or is invalid!");
			return entt::meta_any{};
		}

		if (auto meta_function = meta.func(func_id); meta_function)
			return meta_function.invoke({}, std::forward<Args>(args) ...);

		return entt::meta_any{};
	}

	template <typename ...Args>
	inline auto InvokeMetaFunction(entt::id_type id, entt::id_type func_id, Args&& ...args)
	{
		return InvokeMetaFunction(entt::resolve(id), func_id, std::forward<Args>(args) ...);
	}

}
