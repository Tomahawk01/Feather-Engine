#include "MetaUtilities.h"

entt::id_type Feather::GetIdType(const sol::table& comp)
{
	if (!comp.valid())
	{
		F_FATAL("Failed to get the type id - Component has not been exposed to Lua!");
		assert(comp.valid() && "Failed to get the type id - Component has not been exposed to Lua!");
		return -1;
	}

	const auto func = comp["type_id"].get<sol::function>();
	assert(func.valid() && "type_id() - function has not been exposed to Lua!\n Please ensure all components and types has type_id function\n when creating new user type");

	return func.valid() ? func().get<entt::id_type>() : -1;
}
