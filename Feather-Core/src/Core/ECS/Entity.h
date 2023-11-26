#pragma once

#include "Registry.h"

namespace Feather {

	class Entity
	{
	public:
		Entity(Registry& registry);
		Entity(Registry& registry, const std::string& name = "", const std::string& group = "");
		Entity(Registry& registry, const entt::entity& entity);
		~Entity() = default;

		inline const std::string& GetName() const { return m_Name; }
		inline const std::string& GetGroup() const { return m_Group; }
		inline std::uint32_t Kill() { return m_Registry.GetRegistry().destroy(m_Entity); }
		inline entt::entity& GetEntity() { return m_Entity; }
		inline entt::registry& GetRegistry() { return m_Registry.GetRegistry(); }

		template <typename TComponent, typename ...Args>
		TComponent& AddComponent(Args&& ...args);

		template <typename TComponent, typename ...Args>
		TComponent& ReplaceComponent(Args&& ...args);

		template <typename TComponent>
		TComponent& GetComponent();

		template <typename TComponent>
		bool HasComponent();

		template <typename TComponent>
		void RemoveComponent();

	private:
		Registry& m_Registry;
		entt::entity m_Entity;
		std::string m_Name, m_Group;
	};

}

#include "Entity.inl"
