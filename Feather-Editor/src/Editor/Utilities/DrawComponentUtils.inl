#include "DrawComponentUtils.h"
#include "Core/ECS/Entity.h"

namespace Feather {

	template<typename TComponent>
	inline void DrawComponentsUtil::DrawEntityComponentInfo(Entity& entity)
	{
		auto& component = entity.GetComponent<TComponent>();
		DrawImGuiComponent(component);
	}

	template<typename TComponent>
	inline void DrawComponentsUtil::DrawComponentInfo(TComponent& component)
	{
		DrawImGuiComponent(component);
	}

	template<typename TComponent>
	inline void DrawComponentsUtil::RegisterUIComponent()
	{
		using namespace entt::literals;
		entt::meta<TComponent>()
			.type(entt::type_hash<TComponent>::value())
			.template func<&DrawEntityComponentInfo<TComponent>>("DrawEntityComponentInfo"_hs);
	}

}
