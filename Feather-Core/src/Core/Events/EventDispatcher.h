#pragma once

#include "Logger/Logger.h"

#include <entt.hpp>
#include <sol/sol.hpp>

namespace Feather {

	class EventDispatcher
	{
	public:
		EventDispatcher();
		~EventDispatcher();

		template <typename TEventType, auto Func, typename THandlerType>
		auto AddHandler(THandlerType& handler);

		template <typename TEventType, auto Func, typename THandlerType>
		void RemoveHandler(THandlerType& handler);

		template <typename TEventType>
		void ClearHandlers();

		template <typename TEventType>
		bool HasHandlers();

		template <typename TEventType, auto Func>
		void AddFreeFunc();

		template <typename TEventType>
		void EmitEvent(TEventType& ev);

		template <typename TEventType>
		void EmitEvent(TEventType&& ev);

		template <typename TEventType>
		void EnqueueEvent(TEventType& ev);

		template <typename TEventType>
		void EnqueueEvent(TEventType&& ev);

		template <typename TEventType>
		void UpdateEvent();

		void UpdateAll();

		void ClearQueue();

		static void CreateEventDispatcherLuaBind(sol::state& lua, EventDispatcher& dispatcher);

		template <typename TEvent>
		static void RegisterMetaEventFuncs();

	private:
		std::shared_ptr<entt::dispatcher> m_Dispatcher;
	};

	template <typename TEvent>
	struct LuaHandler
	{
		sol::function callback;
		entt::connection connection;

		void HandleEvent(TEvent& ev)
		{
			if (connection && callback.valid())
				callback(ev);
		}
		~LuaHandler() { F_TRACE("Destroyed!"); }

		void ReleaseConnection()
		{
			connection.release();
			callback.abandon();
		}
	};

	template <typename TEvent>
	void add_handler(EventDispatcher& dispatcher, const sol::table& handler);

	template <typename TEvent>
	void remove_handler(EventDispatcher& dispatcher, const sol::table& handler);

	template <typename TEvent>
	void emit_event(EventDispatcher& dispatcher, const sol::table& event);

	template <typename TEvent>
	void enqueue_event(EventDispatcher& dispatcher, const sol::table& event);

	template <typename TEvent>
	void update_event(EventDispatcher& dispatcher);

	template <typename TEvent>
	bool has_handlers(EventDispatcher& dispatcher);

}

#include "EventDispatcher.inl"
