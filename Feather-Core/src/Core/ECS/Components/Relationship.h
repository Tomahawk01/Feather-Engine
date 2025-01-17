#pragma once

#include <entt.hpp>
#include <sol/sol.hpp>

namespace Feather {

	/* @brief
	* Simple struct for an Unconstrained Hierarchy.
	* The number of children for the parent is not known and is not clamped.
	* Treated like an implicit doubly-linked list with the benefit of
	* no dynamic allocations or reallocations.
	*/
	struct Relationship
	{
		entt::entity self{ entt::null };
		entt::entity firstChild{ entt::null };
		entt::entity prevSibling{ entt::null };
		entt::entity nextSibling{ entt::null };
		entt::entity parent{ entt::null };

		static void CreateRelationshipLuaBind(sol::state& lua);
	};

	class Entity;

	struct RelationshipUtils
	{
		static bool IsAParentOf(Entity& entityA, Entity& entityB);
		static void SetSiblingLinks(Entity& firstChild, Relationship& childRelationship);
		static void RemoveAndDelete(Entity& entityToRemove);
	};

}
