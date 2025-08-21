#include "StateStack.h"

#include "Logger/Logger.h"
#include "Core/States/State.h"

namespace Feather {

	void StateStack::Push(State& state)
	{
		auto hasState = std::ranges::find_if(m_States, [&](const auto& s) { return s.name == state.name; });

		if (hasState == m_States.end())
		{
			state.addState = true;
			m_StateHolder = std::make_unique<State>(state);
			return;
		}

		F_ASSERT(false && "Trying to add a state that is already in the stack!");
	}

	void StateStack::Pop()
	{
		if (m_States.empty())
		{
			F_ERROR("Trying to pop an empty state stack!");
			return;
		}

		auto& top = m_States.back();
		top.killState = true;
	}

	void StateStack::ChangeState(State& state)
	{
		if (!m_States.empty())
			Pop();

		Push(state);
	}

	void StateStack::RemoveState(const std::string& state)
	{
		auto stateItr = std::ranges::find_if(m_States, [&](State& stateObj) { return stateObj.name == state; });
		if (stateItr == m_States.end())
		{
			F_ERROR("Failed to remove state '{}' - Does not exist or is invalid", state);
			return;
		}

		stateItr->killState = true;
	}

	void StateStack::Update(const float dt)
	{
		if (m_StateHolder && m_StateHolder->addState)
		{
			m_States.push_back(*m_StateHolder);

			if (m_StateHolder->on_enter.valid())
			{
				try
				{
					auto result = m_StateHolder->on_enter();
					if (!result.valid())
					{
						sol::error error = result;
						throw error;
					}
				}
				catch (const sol::error& error)
				{
					F_ERROR(error.what());
				}

				m_StateHolder = nullptr;
			}
		}

		if (m_States.empty())
			return;

		auto& topState = m_States.back();
		if (topState.on_update.valid())
		{
			try
			{
				auto result = topState.on_update(dt);
				if (!result.valid())
				{
					sol::error error = result;
					throw error;
				}
			}
			catch (const sol::error& error)
			{
				F_ERROR("Failed to update top state: {}", error.what());
				return;
			}
			catch (const std::exception& ex)
			{
				F_ERROR("Failed to update top state: {}", ex.what());
				return;
			}
			catch (...)
			{
				F_ERROR("Failed to update top state: Unknown Error!");
				return;
			}
		}

		if (topState.handle_inputs.valid())
		{
			try
			{
				auto result = topState.handle_inputs();
				if (!result.valid())
				{
					sol::error error = result;
					throw error;
				}
			}
			catch (const sol::error& error)
			{
				F_ERROR("Failed to handle top state's inputs: {}", error.what());
				return;
			}
		}

		if (topState.killState)
		{
			if (topState.on_exit.valid())
			{
				try
				{
					auto result = topState.on_exit();
					if (!result.valid())
					{
						sol::error error = result;
						throw error;
					}
				}
				catch (const sol::error& error)
				{
					F_ERROR("Failed to exit top state: {}", error.what());
					return;
				}
			}

			m_States.pop_back();
		}
	}

	void StateStack::Render()
	{
		for (const auto& state : m_States)
		{
			if (state.on_render.valid())
			{
				try
				{
					auto result = state.on_render();
					if (!result.valid())
					{
						sol::error error = result;
						throw error;
					}
				}
				catch (const sol::error& error)
				{
					F_ERROR("Failed to render state '{}': {}", state.name, error.what());
				}
			}
		}
	}

	State& StateStack::Top()
	{
		F_ASSERT(!m_States.empty() && "Can not get the top of an empty stack");

		if (m_States.empty())
			throw std::runtime_error("State stack is empty!");

		return m_States.back();
	}

	void StateStack::CreateLuaStateStackBind(sol::state& lua)
	{
		lua.new_usertype<StateStack>(
			"StateStack",
			sol::call_constructor,
			sol::constructors<StateStack()>(),
			"changeState", &StateStack::ChangeState,
			"push", &StateStack::Push,
			"pop", &StateStack::Pop,
			"update", &StateStack::Update,
			"render", &StateStack::Render,
			"top", &StateStack::Top
		);
	}
}
