#include "ECSUtils.h"

#include "Components/AllComponents.h"
#include "Entity.h"

namespace Feather {

	entt::entity FindEntityByTag(Registry& registry, const std::string& tag)
	{
        auto ids = registry.GetRegistry().view<Identification>(entt::exclude<TileComponent>);

        auto parItr = std::ranges::find_if(ids, [&](const auto& e) {
            Entity en{ &registry, e };
            return en.GetName() == tag;
        });

        if (parItr != ids.end())
            return *parItr;

        return entt::entity{ entt::null };
	}

}
