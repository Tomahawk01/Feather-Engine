#include "StateMachine.h"
#include "Logger/Logger.h"

namespace Feather {

	StateMachine::StateMachine()
		: StateMachine(sol::lua_nil_t{})
	{}

	StateMachine::StateMachine(const sol::table& stateFuncs)
		: m_mapStates{}, m_CurrentState{""}, m_StateTable{stateFuncs}
	{}

	void StateMachine::ChangeState(const std::string& stateName, bool removeState, const sol::object& enterParams)
	{
		auto stateItr = m_mapStates.find(stateName);
		if (stateItr == m_mapStates.end())
		{
			F_ERROR("Failed to change states. '{}' does not exists", stateName);
			return;
		}

		auto& newState = stateItr->second;
		if (m_CurrentState.empty())
		{
			m_CurrentState = stateName;
		}
		else
		{
			auto& oldState = m_mapStates.at(m_CurrentState);
			if (oldState->on_exit.valid())
			{
				try
				{
					auto result = oldState->on_exit();
					if (!result.valid())
					{
						sol::error error = result;
						throw error;
					}
				}
				catch (const sol::error& error)
				{
					F_ERROR("Failed to exit state: {}", error.what());
					return;
				}
			}

			if (removeState)
				oldState->killState = true;

			m_CurrentState = stateName;
		}

		if (newState->on_enter.valid())
		{
			try
			{
				auto result = newState->on_enter(enterParams);
				if (!result.valid())
				{
					sol::error error = result;
					throw error;
				}
			}
			catch (const sol::error& error)
			{
				F_ERROR("Failed to enter state: {}", error.what());
				return;
			}
		}
	}

	void StateMachine::Update(const float dt)
	{
		try
		{
			auto stateItr = m_mapStates.find(m_CurrentState);
			if (stateItr == m_mapStates.end())
				return;

			if (stateItr->second->on_update.valid())
			{
				auto result = stateItr->second->on_update(dt);
				if (!result.valid())
				{
					sol::error error = result;
					throw error;
				}
			}

			// Check for states to remove
			std::erase_if(m_mapStates, [](auto& state) { return state.second->killState; });
		}
		catch (const sol::error& error)
		{
			F_ERROR("Failed to update state: {}", error.what());
		}
		catch (...)
		{
			F_ERROR("Failed to update state: Unknown error");
		}
	}

	void StateMachine::Render()
	{
		try
		{
			auto stateItr = m_mapStates.find(m_CurrentState);
			if (stateItr == m_mapStates.end())
				return;

			if (stateItr->second->on_render.valid())
			{
				auto result = stateItr->second->on_render();
				if (!result.valid())
				{
					sol::error error = result;
					throw error;
				}
			}
		}
		catch (const sol::error& error)
		{
			F_ERROR("Failed to render state: {}", error.what());
		}
		catch (...)
		{
			F_ERROR("Failed to render state: Unknown error");
		}
	}

	void StateMachine::AddState(const State& state)
	{
		if (m_mapStates.contains(state.name))
		{
			F_ERROR("Failed to add state: '{}' already exists", state.name);
			return;
		}

		m_mapStates.emplace(state.name, std::make_shared<State>(state));
	}

	void StateMachine::ExitState()
	{
		auto stateItr = m_mapStates.find(m_CurrentState);
		if (stateItr == m_mapStates.end())
		{
			F_ERROR("Failed to exit state: '{}' does not exist", m_CurrentState);
			return;
		}

		stateItr->second->on_exit();
		stateItr->second->killState = true;
		m_CurrentState.clear();
	}

	void StateMachine::DestroyStates()
	{
		for (auto& [name, state] : m_mapStates)
		{
			state->on_exit();
		}

		m_mapStates.clear();
	}

	void StateMachine::CreateLuaStateMachineBind(sol::state& lua)
	{
		lua.new_usertype<StateMachine>(
			"StateMachine",
			sol::call_constructor,
			sol::constructors<StateMachine(), StateMachine(const sol::table&)>(),
			"changeState", sol::overload(
				[](StateMachine& sm, const std::string& state, bool remove, const sol::object& enterParams) {
					sm.ChangeState(state, remove, enterParams);
				},
				[](StateMachine& sm, const std::string& state, bool remove) {
					sm.ChangeState(state, remove);
				},
				[](StateMachine& sm, const std::string& state) {
					sm.ChangeState(state);
				}
			),
			"update", &StateMachine::Update,
			"render", &StateMachine::Render,
			"currentState", &StateMachine::CurrentState,
			"addState", &StateMachine::AddState,
			"exitState", &StateMachine::ExitState,
			"destroy", &StateMachine::DestroyStates
		);
	}

}
