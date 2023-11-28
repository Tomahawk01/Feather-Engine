#pragma once

#include <memory>
#include <sol/sol.hpp>

#include "Windowing/Input/Keyboard.h"
#include "Windowing/Input/Mouse.h"

namespace Feather {

	class InputManager
	{
	public:
		static InputManager& GetInstance();
		static void CreateLuaInputBindings(sol::state& lua);

		inline Keyboard& GetKeyboard() { return *m_Keyboard; }
		inline Mouse& GetMouse() { return *m_Mouse; }

	private:
		std::unique_ptr<Keyboard> m_Keyboard;
		std::unique_ptr<Mouse> m_Mouse;
	private:
		InputManager();
		~InputManager() = default;
		InputManager(const InputManager&) = delete;
		InputManager& operator=(const InputManager&) = delete;

		static void RegisterLuaKeyNames(sol::state& lua);
		static void RegisterLuaMouseButtonNames(sol::state& lua);
	};

}
