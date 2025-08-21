#include "State.h"

#include "Logger/Logger.h"

void Feather::State::CreateLuaStateBind(sol::state& lua)
{
	lua.new_usertype<State>(
		"State",
		sol::call_constructor,
		sol::factories(
			[](const std::string& name) {
				return new State{ .name = name };
			},
			[](const std::string& name, sol::protected_function on_enter, sol::protected_function on_exit,
			   sol::protected_function on_update, sol::protected_function on_render, sol::protected_function handle_inputs, sol::object variables) {
				return State{
					.name = name,
					.on_render = on_render,
					.on_update = on_update,
					.on_enter = on_enter,
					.on_exit = on_exit,
					.handle_inputs = handle_inputs,
					.variables = variables
				};
			}
		),
		"setOnEnter", [](State& state, sol::protected_function on_enter) {
			if (!on_enter.valid())
			{
				F_ERROR("Failed to set on_enter function");
				return;
			}
			state.on_enter = on_enter;
		},
		"setOnExit", [](State& state, sol::protected_function on_exit) {
			if (!on_exit.valid())
			{
				F_ERROR("Failed to set on_exit function");
				return;
			}
			state.on_exit = on_exit;
		},
		"setOnUpdate", [](State& state, sol::protected_function on_update) {
			if (!on_update.valid())
			{
				F_ERROR("Failed to set on_update function");
				return;
			}
			state.on_update = on_update;
		},
		"setOnRender", [](State& state, sol::protected_function on_render) {
			if (!on_render.valid())
			{
				F_ERROR("Failed to set on_render function");
				return;
			}
			state.on_render = on_render;
		},
		"setHandleInputs", [](State& state, sol::protected_function handle_inputs) {
			if (!handle_inputs.valid())
			{
				F_ERROR("Failed to set handle_inputs function");
				return;
			}
			state.handle_inputs = handle_inputs;
		},
		"setVariableTable", [](State& state, const sol::table& table) {
			if (!table.valid())
			{
				F_ERROR("Failed to set variables");
				return;
			}
			state.variables = table;
		},
		"variables", &State::variables,
		"name", &State::name,
		"killState", &State::killState
	);
}
