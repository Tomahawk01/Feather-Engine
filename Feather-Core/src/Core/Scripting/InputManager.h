#pragma once

#include <memory>
#include <sol/sol.hpp>

#include "Windowing/Input/Keyboard.h"
#include "Windowing/Input/Mouse.h"
#include "Windowing/Input/Gamepad.h"

constexpr int MAX_CONTROLLERS = 4;

namespace Feather {

	class InputManager
	{
	public:
		static InputManager& GetInstance();
		static void CreateLuaInputBindings(sol::state& lua);

		inline Keyboard& GetKeyboard() { return *m_Keyboard; }
		inline Mouse& GetMouse() { return *m_Mouse; }
		inline std::map<int, std::shared_ptr<Gamepad>>& GetControllers() { return m_GameControllers; }

		std::shared_ptr<Gamepad> GetController(int index);

		bool AddGamepad(Sint32 gamepadID);
		bool RemoveGamepad(Sint32 gamepadID);
		void GamepadButtonPressed(const SDL_Event& event);
		void GamepadButtonReleased(const SDL_Event& event);
		void GamepadAxisValues(const SDL_Event& event);
		void GamepadHatValues(const SDL_Event& event);
		void UpdateGamepads();

	private:
		std::unique_ptr<Keyboard> m_Keyboard;
		std::unique_ptr<Mouse> m_Mouse;
		std::map<int, std::shared_ptr<Gamepad>> m_GameControllers;
	private:
		InputManager();
		~InputManager() = default;
		InputManager(const InputManager&) = delete;
		InputManager& operator=(const InputManager&) = delete;

		static void RegisterLuaKeyNames(sol::state& lua);
		static void RegisterLuaMouseButtonNames(sol::state& lua);
		static void RegisterLuaGamepadButtonNames(sol::state& lua);
	};

}
