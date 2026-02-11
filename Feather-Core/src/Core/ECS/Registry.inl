#include "Registry.h"

namespace Feather {

	template<typename TContext>
	inline TContext Registry::AddToContext(TContext context)
	{
		return m_Registry->ctx().emplace<TContext>(context);
	}

	template<typename TContext>
	inline TContext& Registry::GetContext()
	{
		return m_Registry->ctx().get<TContext>();
	}

	template<typename TContext>
	inline TContext* Registry::TryGetContext()
	{
		return m_Registry->ctx().find<TContext>();
	}

	template<typename TContext>
	inline bool Registry::RemoveContext()
	{
		return m_Registry->ctx().erase<TContext>();
	}

	template<typename TContext>
	inline bool Registry::HasContext()
	{
		return m_Registry->ctx().contains<TContext>();
	}

	template<typename... Excludes>
	inline void Registry::DestroyEntities()
	{
		auto view = m_Registry->view<entt::entity>(entt::exclude<Excludes...>);
		for (auto entity : view)
		{
			m_Registry->destroy(entity);
		}
	}

	template <typename TComponent>
	entt::runtime_view& add_component_to_view(Registry* registry, entt::runtime_view& view)
	{
		return view.iterate(registry->GetRegistry().storage<TComponent>());
	}

	template <typename TComponent>
	auto exclude_component_from_view(Registry* registry, entt::runtime_view* view)
	{
		view->exclude(registry->GetRegistry().storage<TComponent>());
	}

	template <typename TComponent>
	void Registry::RegisterMetaComponent()
	{
		using namespace entt::literals;
		entt::meta<TComponent>()
			.type(entt::type_hash<TComponent>::value())
			.template func<&add_component_to_view<TComponent>>("add_component_to_view"_hs)
			.template func<&exclude_component_from_view<TComponent>>("exclude_component_from_view"_hs);
	}

}
