#include "Relationship.h"

#include "Core/ECS/Entity.h"

namespace Feather {

	void Relationship::CreateRelationshipLuaBind(sol::state& lua)
	{
		lua.new_usertype<Relationship>(
			"Relationship",
			"type_id",
			&entt::type_hash<Relationship>::value,
			sol::call_constructor,
			sol::constructors<Relationship()>(),
			"self",
			[](Relationship& relations) { return static_cast<uint32_t>(relations.self); },
			"parent",
			[](Relationship& relations) { return static_cast<uint32_t>(relations.parent); },
			"nextSibling",
			[](Relationship& relations) { return static_cast<uint32_t>(relations.nextSibling); },
			"prevSibling",
			[](Relationship& relations) { return static_cast<uint32_t>(relations.prevSibling); },
			"firstChild",
			[](Relationship& relations) { return static_cast<uint32_t>(relations.firstChild); });
	}

	bool RelationshipUtils::IsAParentOf(Entity& entityA, Entity& entityB)
	{
		auto& registry = entityA.GetEnttRegistry();
		auto entity = entityB.GetEntity();
		const auto& relations = entityA.GetComponent<Relationship>();

		auto parentEnt = relations.parent;

		while (parentEnt != entt::null)
		{
			if (parentEnt == entity)
				return true;

			const auto& parentRelations = registry.get<Relationship>(parentEnt);
			parentEnt = parentRelations.parent;
		}

		return false;
	}

	void RelationshipUtils::SetSiblingLinks(Entity& firstChild, Relationship& childRelationship)
	{
		auto& registry = firstChild.GetEnttRegistry();
		auto parentChild = registry.get<Relationship>(firstChild.GetEntity());

		while (parentChild.nextSibling != entt::null)
		{
			parentChild = registry.get<Relationship>(parentChild.nextSibling);
		}

		auto& prevSibling = registry.get<Relationship>(parentChild.self);
		prevSibling.nextSibling = childRelationship.self;
		childRelationship.prevSibling = prevSibling.self;
	}

	void RelationshipUtils::RemoveAndDelete(Entity& entityToRemove, std::vector<std::string>& entitiesRemoved)
	{
		auto& registry = entityToRemove.GetEnttRegistry();
		auto& Reg = entityToRemove.GetRegistry();
		auto& relations = entityToRemove.GetComponent<Relationship>();

		// First we need to delete all of the children
		if (relations.firstChild != entt::null)
		{
			std::vector<entt::entity> childrenToDelete{};
			childrenToDelete.push_back(relations.firstChild);
			auto childRelations = registry.get<Relationship>(relations.firstChild);
			while (childRelations.nextSibling != entt::null)
			{
				childrenToDelete.push_back(childRelations.nextSibling);
				childRelations = registry.get<Relationship>(childRelations.nextSibling);
			}

			for (auto entity : childrenToDelete)
			{
				Entity ent{ &Reg, entity };
				RemoveAndDelete(ent, entitiesRemoved);
			}
		}

		// Now we need to fix any links to the entity
		if (relations.parent != entt::null)
		{
			auto& parent = registry.get<Relationship>(relations.parent);

			// If the child is the first child, grab the next child and adjust links
			if (parent.firstChild == relations.self)
			{
				parent.firstChild = relations.nextSibling;
			}
			else
			{
				// Handle the child and its siblings
				if (relations.prevSibling != entt::null)
				{
					auto& prev = registry.get<Relationship>(relations.prevSibling);
					prev.nextSibling = relations.nextSibling;
				}
				if (relations.nextSibling != entt::null)
				{
					auto& next = registry.get<Relationship>(relations.nextSibling);
					next.prevSibling = relations.prevSibling;
				}
			}

			relations.prevSibling = entt::null;
			relations.nextSibling = entt::null;
		}

		// Now that the links have been adjusted, we can delete the entity
		//F_TRACE("Just Destroyed: {}", static_cast<std::uint32_t>(entityToRemove.GetEntity()));
		entitiesRemoved.emplace_back(entityToRemove.GetName());
		entityToRemove.Destroy();
	}

	entt::entity RelationshipUtils::DuplicateRecursive(entt::registry& reg, entt::entity source, entt::entity newParent)
	{
		// Create a new entity
		entt::entity copy = reg.create();

		// Copy all components except relationship
		for (auto&& [id, storage] : reg.storage())
		{
			if (id == entt::type_hash<Relationship>::value())
				continue;

			if (storage.contains(source))
			{
				storage.push(copy, storage.value(source));
			}
		}

		// Add a new relationship
		auto& copyRel = reg.emplace<Relationship>(copy);
		copyRel.self = copy;
		copyRel.parent = newParent;

		// Recursively duplicate children
		const auto& srcRel = reg.get<Relationship>(source);
		for (entt::entity child = srcRel.firstChild;
			 child != entt::null;
			 child = reg.get<Relationship>(child).nextSibling)
		{
			entt::entity childCopy = DuplicateRecursive(reg, child, copy);

			// Append to copy's child list
			auto& copyRelRef = reg.get<Relationship>(copy);
			auto& childRel = reg.get<Relationship>(childCopy);

			if (copyRelRef.firstChild == entt::null)
			{
				copyRelRef.firstChild = childCopy;
			}
			else
			{
				entt::entity last = copyRelRef.firstChild;
				auto* lastRel = reg.try_get<Relationship>(last);
				while (lastRel && lastRel->nextSibling != entt::null)
				{
					last = lastRel->nextSibling;
					lastRel = reg.try_get<Relationship>(last);
				}

				if (lastRel)
				{
					lastRel->nextSibling = childCopy;
				}

				childRel.prevSibling = last;
			}
		}

		return copy;
	}

}
