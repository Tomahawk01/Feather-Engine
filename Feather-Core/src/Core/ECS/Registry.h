#pragma once

#include <entt.hpp>

namespace Feather {
	
	class Registry
	{
	public:
		Registry();
		~Registry() = default;

		inline entt::registry& GetRegistry() { return *m_Registry; }
		inline entt::entity CreateEntity() { return m_Registry->create(); }

		template <typename TContext>
		TContext AddToContext(TContext context);

		template <typename TContext>
		TContext& GetContext();

	private:
		std::unique_ptr<entt::registry> m_Registry;
	};

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

}
