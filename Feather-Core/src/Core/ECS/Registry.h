#pragma once

#include <entt.hpp>
#include <sol/sol.hpp>

namespace Feather {
	
	class Registry
	{
	public:
		Registry();
		~Registry() = default;

		inline entt::registry& GetRegistry() { return *m_Registry; }
		inline entt::entity CreateEntity() { return m_Registry->create(); }
		inline void ClearRegistry() { m_Registry->clear(); }

		template <typename TContext>
		TContext AddToContext(TContext context);

		template <typename TContext>
		TContext& GetContext();

		template <typename TContext>
		TContext* TryGetContext();

		template <typename TContext>
		bool RemoveContext();

		template <typename TContext>
		bool HasContext();

		static void CreateLuaRegistryBind(sol::state& lua, Registry& registry);

		template <typename TComponent>
		static void RegisterMetaComponent();

	private:
		std::unique_ptr<entt::registry> m_Registry;
	};

	template <typename TComponent>
	entt::runtime_view& add_component_to_view(Registry* registry, entt::runtime_view& view);

	template <typename TComponent>
	entt::runtime_view& exclude_component_from_view(Registry* registry, entt::runtime_view& view);

}

#include "Registry.inl"
