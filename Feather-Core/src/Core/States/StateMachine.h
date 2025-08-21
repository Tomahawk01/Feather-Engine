#pragma once

#include <sol/sol.hpp>

namespace Feather {

	struct State;

	class StateMachine
	{
	public:
		StateMachine();
		StateMachine(const sol::table& stateFuncs);

		void ChangeState(const std::string& stateName, bool removeState = false, const sol::object& enterParams = sol::lua_nil_t{});

		void Update(const float dt);
		void Render();
		void AddState(const State& state);
		void ExitState();
		void DestroyStates();
		const std::string& CurrentState() const { return m_CurrentState; }

		static void CreateLuaStateMachineBind(sol::state& lua);

	private:
		std::map<std::string, std::shared_ptr<State>> m_mapStates;
		std::string m_CurrentState;
		std::optional<sol::table> m_StateTable;
	};

}