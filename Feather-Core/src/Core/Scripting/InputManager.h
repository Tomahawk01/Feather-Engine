#pragma once

#include <memory>
#include <sol/sol.hpp>

#include "Windowing/Input/Keyboard.h"

namespace Feather {

	class InputManager
	{
	public:
		static InputManager& GetInstance();
		static void CreateLuaInputBindings(sol::state& lua);

		inline Keyboard& GetKeyboard() { return *m_Keyboard; }

	private:
		std::unique_ptr<Keyboard> m_Keyboard;
	private:
		InputManager();
		~InputManager() = default;
		InputManager(const InputManager&) = delete;
		InputManager& operator=(const InputManager&) = delete;

		static void RegisterLuaKeyNames(sol::state& lua);
	};

}
