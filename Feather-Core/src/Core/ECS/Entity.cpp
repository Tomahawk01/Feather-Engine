#include "Entity.h"

#include "Components/AllComponents.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Core/Scene/Scene.h"
#include "MetaUtilities.h"

namespace Feather {

	Entity::Entity(Registry& registry)
		: Entity(registry, "GameObject", "")
	{}

	Entity::Entity(Registry& registry, const std::string& name, const std::string& group)
		: m_Registry(registry), m_Entity{ registry.CreateEntity() }, m_Name{ name }, m_Group{ group }
	{
		AddComponent<Identification>(Identification{
									 .name = name,
									 .group = group,
									 .entity_id = static_cast<uint32_t>(m_Entity)});

		AddComponent<Relationship>(Relationship{ .self = m_Entity });
	}

	Entity::Entity(Registry& registry, const entt::entity& entity)
		: m_Registry(registry), m_Entity(entity), m_Name{}, m_Group{}
	{
		if (HasComponent<Identification>())
		{
			auto id = GetComponent<Identification>();
			m_Name = id.name;
			m_Group = id.group;
		}
	}

	bool Entity::AddChild(entt::entity child, bool isSetLocal)
	{
		auto& registry = m_Registry.GetRegistry();
		auto& relations = registry.get<Relationship>(m_Entity);

		Entity childEntity{ m_Registry, child };
		auto& childRelationship = childEntity.GetComponent<Relationship>();

		if (RelationshipUtils::IsAParentOf(*this, childEntity))
		{
			F_ERROR("Failed to add child. Cannot make a parent into a child");
			return false;
		}

		// Check to see if the parent is already this entity
		if (childRelationship.parent == m_Entity)
		{
			// If the child is the first child, grab the next child and adjust links
			if (relations.firstChild == child)
			{
				relations.firstChild = childRelationship.nextSibling;
			}
			else
			{
				// Handle the child and its siblings
				if (childRelationship.prevSibling != entt::null)
				{
					auto& prev = registry.get<Relationship>(childRelationship.prevSibling);
					prev.nextSibling = childRelationship.nextSibling;
				}
				if (childRelationship.nextSibling != entt::null)
				{
					auto& next = registry.get<Relationship>(childRelationship.nextSibling);
					next.prevSibling = childRelationship.prevSibling;
				}
			}

			// Reset the child's siblings
			childRelationship.prevSibling = entt::null;
			childRelationship.nextSibling = entt::null;

			childRelationship.parent = relations.parent;
			if (auto* parent = registry.try_get<Relationship>(relations.parent))
			{
				if (parent->firstChild != entt::null)
				{
					Entity firstChild{ m_Registry, parent->firstChild };
					RelationshipUtils::SetSiblingLinks(firstChild, childRelationship);
				}
			}

			// Set the childs local position
			auto& childTransform = childEntity.GetComponent<TransformComponent>();
			if (relations.parent != entt::null && isSetLocal)
			{
				childTransform.localPosition = childTransform.position - registry.get<TransformComponent>(relations.parent).position;
				childTransform.localRotation = childTransform.rotation;
			}
			return true;
		}

		// Handle the child and its siblings
		if (childRelationship.prevSibling != entt::null)
		{
			if (auto* prev = registry.try_get<Relationship>(childRelationship.prevSibling))
			{
				prev->nextSibling = childRelationship.nextSibling;
			}
		}

		if (childRelationship.nextSibling != entt::null)
		{
			if (auto* next = registry.try_get<Relationship>(childRelationship.nextSibling))
			{
				next->prevSibling = childRelationship.prevSibling;
			}
		}

		// Remove the child count from the parent
		if (childRelationship.parent != entt::null)
		{
			auto& parent = registry.get<Relationship>(childRelationship.parent);
			if (parent.firstChild == child)
			{
				parent.firstChild = entt::null;
				// Get the next sibling and set the links
				if (auto* nextSibling = registry.try_get<Relationship>(childRelationship.nextSibling))
				{
					parent.firstChild = childRelationship.nextSibling;
					nextSibling->prevSibling = entt::null;
				}
			}
		}

		// Reset the child's siblings, they will change later
		childRelationship.nextSibling = entt::null;
		childRelationship.prevSibling = entt::null;

		// Set the parent to the new entity
		childRelationship.parent = m_Entity;

		// Set the childs local position
		auto& childTransform = childEntity.GetComponent<TransformComponent>();
		if (isSetLocal)
		{
			childTransform.localPosition = childTransform.position - GetComponent<TransformComponent>().position;
			childTransform.localRotation = childTransform.rotation;
		}

		// Check to see if the parent has any children.
		// Parent has no children, add as the first child
		if (relations.firstChild == entt::null)
		{
			relations.firstChild = child;
		}
		else // If the parent already has a child, we need to find the last location and set
		{
			// We want to get a copy of the parentChild here so we can move to the next one
			Entity firstChild{ m_Registry, relations.firstChild };
			RelationshipUtils::SetSiblingLinks(firstChild, childRelationship);
		}

		return true;
	}

	void Entity::UpdateTransform()
	{
		auto& relations = GetComponent<Relationship>();
		auto& transform = GetComponent<TransformComponent>();

		//glm::vec2 parentPosition{ 0.0f };
		auto parent = relations.parent;
		if (parent != entt::null)
		{
			Entity ent{ m_Registry, parent };
			const auto& parentTransform = ent.GetComponent<TransformComponent>();
			transform.position = parentTransform.position + transform.localPosition;
			transform.rotation = parentTransform.rotation + transform.localRotation;
		}

		if (relations.firstChild == entt::null)
			return;

		auto child = relations.firstChild;
		while (child != entt::null)
		{
			Entity childEnt{ m_Registry, child };
			childEnt.UpdateTransform();
			child = childEnt.GetComponent<Relationship>().nextSibling;
		}
	}

	void Entity::ChangeName(const std::string& name)
	{
		auto& id = GetComponent<Identification>();
		id.name = name;
		m_Name = name;
	}

	std::uint32_t Entity::Kill()
	{
		if (!m_Registry.IsValid(m_Entity))
		{
			F_ERROR("Failed to destroy entity. Entity ID '{}' is not valid", static_cast<std::uint32_t>(m_Entity));
			return static_cast<std::uint32_t>(entt::null);
		}

		return m_Registry.GetRegistry().destroy(m_Entity);
	}

	void Entity::CreateLuaEntityBind(sol::state& lua, Registry& registry)
	{
		using namespace entt::literals;
		lua.new_usertype<Entity>(
			"Entity",
			sol::call_constructor,
			sol::factories(
				[&](Registry& reg, const std::string& name, const std::string& group)
				{
					return Entity{ reg, name, group };
				},
				[&](const std::string& name, const std::string& group)
				{
					return Entity{ registry, name, group };
				},
				[&](uint32_t id)
				{
					return Entity{ registry, static_cast<entt::entity>(id) };
				}
			),
			"addComponent", [](Entity& entity, const sol::table& comp, sol::this_state s) -> sol::object
			{
				if (!comp.valid())
					return sol::lua_nil_t{};

				const auto component = InvokeMetaFunction(
					GetIdType(comp),
					"add_component"_hs,
					entity,
					comp,
					s);

				return component ? component.cast<sol::reference>() : sol::lua_nil_t{};
			},
			"hasComponent", [](Entity& entity, const sol::table& comp)
			{
				const auto has_comp = InvokeMetaFunction(
					GetIdType(comp),
					"has_component"_hs,
					entity);

				return has_comp ? has_comp.cast<bool>() : false;
			},
			"getComponent", [](Entity& entity, const sol::table& comp, sol::this_state s)
			{
				const auto component = InvokeMetaFunction(
					GetIdType(comp),
					"get_component"_hs,
					entity,
					s);

				return component ? component.cast<sol::reference>() : sol::lua_nil_t{};
			},
			"removeComponent", [](Entity& entity, const sol::table& comp)
			{
				const auto component = InvokeMetaFunction(
					GetIdType(comp),
					"remove_component"_hs,
					entity);

				return component ? component.cast<sol::reference>() : sol::lua_nil_t{};
			},
			"name", &Entity::GetName,
			"group", &Entity::GetGroup,
			"kill", &Entity::Kill,
			"addChild", [](Entity& entity, Entity& child) { entity.AddChild(child.GetEntity()); },
			"updateTransform", &Entity::UpdateTransform,
			"updateIsoSorting",
			[](Entity& entity, const Canvas& canvas)
			{
				if (auto* sprite = entity.TryGetComponent<SpriteComponent>(); sprite->isIsometric)
				{
					auto& transform = entity.GetComponent<TransformComponent>();
					auto [cellX, cellY] = ConvertWorldPosToIsoCoords(transform.position, canvas);
					sprite->isoCellX = cellX;
					sprite->isoCellY = cellY;
				}
				else
				{
					F_ERROR("Entity does not have a sprite component or is not using iso sorting");
				}
			},
			"id", [](Entity& entity) { return static_cast<uint32_t>(entity.GetEntity()); }
		);
	}

}
