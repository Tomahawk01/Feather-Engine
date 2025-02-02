#pragma once
#include <entt.hpp>

namespace Feather {

	class EventDispatcher
	{
	public:
		EventDispatcher() = default;
		~EventDispatcher() = default;

		template <typename TEventType, auto Func, typename THandlerType>
		void AddHandler(THandlerType& handler)
		{
			m_Dispatcher.sink<TEventType>().template connect<Func>(handler);
		}

		template <typename TEventType, auto Func, typename THandlerType>
		void RemoveHandler(THandlerType& handler)
		{
			m_Dispatcher.sink<TEventType>().template disconnect<Func>(handler);
		}

		template <typename TEventType>
		void ClearHandlers()
		{
			m_Dispatcher.sink<TEventType>().disconnect();
		}

		template <typename TEventType, auto Func>
		void AddFreeFunc()
		{
			m_Dispatcher.sink<TEventType>().template connect<Func>();
		}

		template <typename TEventType>
		void EmitEvent(TEventType& ev)
		{
			m_Dispatcher.trigger(ev);
		}

		template <typename TEventType>
		void EmitEvent(TEventType&& ev)
		{
			m_Dispatcher.trigger(ev);
		}

		template <typename TEventType>
		void EnqueueEvent(TEventType& ev)
		{
			m_Dispatcher.enqueue<TEventType>(ev);
		}

		template <typename TEventType>
		void EnqueueEvent(TEventType&& ev)
		{
			m_Dispatcher.enqueue<TEventType>(ev);
		}

		template <typename TEventType>
		void UpdateEvent()
		{
			m_Dispatcher.update<TEventType>();
		}

		void UpdateAll() { m_Dispatcher.update(); }

	private:
		entt::dispatcher m_Dispatcher;
	};

}
