#include "InputManager.h"

namespace Feather {

    InputManager::InputManager()
        : m_Keyboard{std::make_unique<Keyboard>()}
    {}

	InputManager& InputManager::GetInstance()
	{
		static InputManager instance{};
		return instance;
	}

	void InputManager::CreateLuaInputBindings(sol::state& lua)
	{
        RegisterLuaKeyNames(lua);

        auto& inputManager = GetInstance();
        auto& keyboard = inputManager.GetKeyboard();

        lua.new_usertype<Keyboard>(
            "Keyboard",
            sol::no_constructor,
            "just_pressed", [&](int key) { return keyboard.IsKeyJustPressed(key); },
            "just_released", [&](int key) { return keyboard.IsKeyJustReleased(key); },
            "pressed", [&](int key) { return keyboard.IsKeyPressed(key); }
        );
	}

	void InputManager::RegisterLuaKeyNames(sol::state& lua)
	{
        lua.set("KEY_A", F_KEY_A);
        lua.set("KEY_B", F_KEY_B);
        lua.set("KEY_C", F_KEY_C);
        lua.set("KEY_D", F_KEY_D);
        lua.set("KEY_E", F_KEY_E);
        lua.set("KEY_F", F_KEY_F);
        lua.set("KEY_G", F_KEY_G);
        lua.set("KEY_H", F_KEY_H);
        lua.set("KEY_I", F_KEY_I);
        lua.set("KEY_J", F_KEY_J);
        lua.set("KEY_K", F_KEY_K);
        lua.set("KEY_L", F_KEY_L);
        lua.set("KEY_M", F_KEY_M);
        lua.set("KEY_N", F_KEY_N);
        lua.set("KEY_O", F_KEY_O);
        lua.set("KEY_P", F_KEY_P);
        lua.set("KEY_Q", F_KEY_Q);
        lua.set("KEY_R", F_KEY_R);
        lua.set("KEY_S", F_KEY_S);
        lua.set("KEY_T", F_KEY_T);
        lua.set("KEY_U", F_KEY_U);
        lua.set("KEY_V", F_KEY_V);
        lua.set("KEY_W", F_KEY_W);
        lua.set("KEY_X", F_KEY_X);
        lua.set("KEY_Y", F_KEY_Y);
        lua.set("KEY_Z", F_KEY_Z);

        lua.set("KEY_0", F_KEY_0);
        lua.set("KEY_1", F_KEY_1);
        lua.set("KEY_2", F_KEY_2);
        lua.set("KEY_3", F_KEY_3);
        lua.set("KEY_4", F_KEY_4);
        lua.set("KEY_5", F_KEY_5);
        lua.set("KEY_6", F_KEY_6);
        lua.set("KEY_7", F_KEY_7);
        lua.set("KEY_8", F_KEY_8);
        lua.set("KEY_9", F_KEY_9);

        lua.set("KEY_ENTER", F_KEY_RETURN);
        lua.set("KEY_BACKSPACE", F_KEY_BACKSPACE);
        lua.set("KEY_ESC", F_KEY_ESCAPE);
        lua.set("KEY_SPACE", F_KEY_SPACE);
        lua.set("KEY_LCTRL", F_KEY_LCTRL);
        lua.set("KEY_RCTRL", F_KEY_RCTRL);
        lua.set("KEY_LALT", F_KEY_LALT);
        lua.set("KEY_RALT", F_KEY_RALT);
        lua.set("KEY_LSHIFT", F_KEY_LSHIFT);
        lua.set("KEY_RSHIFT", F_KEY_RSHIFT);

        lua.set("KEY_F1", F_KEY_F1);
        lua.set("KEY_F2", F_KEY_F2);
        lua.set("KEY_F3", F_KEY_F3);
        lua.set("KEY_F4", F_KEY_F4);
        lua.set("KEY_F5", F_KEY_F5);
        lua.set("KEY_F6", F_KEY_F6);
        lua.set("KEY_F7", F_KEY_F7);
        lua.set("KEY_F8", F_KEY_F8);
        lua.set("KEY_F9", F_KEY_F9);
        lua.set("KEY_F10", F_KEY_F10);
        lua.set("KEY_F11", F_KEY_F11);
        lua.set("KEY_F12", F_KEY_F12);

        lua.set("KEY_UP", F_KEY_UP);
        lua.set("KEY_RIGHT", F_KEY_RIGHT);
        lua.set("KEY_DOWN", F_KEY_DOWN);
        lua.set("KEY_LEFT", F_KEY_LEFT);

        lua.set("KP_KEY_0", F_KEY_KP0);
        lua.set("KP_KEY_1", F_KEY_KP1);
        lua.set("KP_KEY_2", F_KEY_KP2);
        lua.set("KP_KEY_3", F_KEY_KP3);
        lua.set("KP_KEY_4", F_KEY_KP4);
        lua.set("KP_KEY_5", F_KEY_KP5);
        lua.set("KP_KEY_6", F_KEY_KP6);
        lua.set("KP_KEY_7", F_KEY_KP7);
        lua.set("KP_KEY_8", F_KEY_KP8);
        lua.set("KP_KEY_9", F_KEY_KP9);
        lua.set("KP_KEY_ENTER", F_KEY_KP_ENTER);
	}

}
