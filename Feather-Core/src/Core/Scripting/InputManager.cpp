#include "InputManager.h"

#include "Logger/Logger.h"
#include "Windowing/Input/Keyboard.h"
#include "Windowing/Input/Mouse.h"
#include "Windowing/Input/Gamepad.h"
#include "Renderer/Core/Camera2D.h"

#include <glm/glm.hpp>

namespace Feather {

    InputManager::InputManager()
        : m_Keyboard{ std::make_unique<Keyboard>() }, m_Mouse{ std::make_unique<Mouse>() }
    {
        // Load already connected devices
        for (int i = 0; i < SDL_NumJoysticks(); ++i)
        {
            SDL_GameController* controller = SDL_GameControllerOpen(i);
            if (controller)
            {
                std::shared_ptr<Gamepad> gamepad{ nullptr };
                try
                {
                    gamepad = std::make_shared<Gamepad>(std::move(MakeSharedFromSDLType<Controller>(controller)));
                }
                catch (...)
                {
                    std::string error{ SDL_GetError() };
                    F_ERROR("Failed to open gamepad device: {}", error);
                    continue;
                }

                for (int j = 1; j <= MAX_CONTROLLERS; j++)
                {
                    if (m_GameControllers.contains(j))
                        continue;

                    m_GameControllers.emplace(j, std::move(gamepad));
                    F_TRACE("Gamepad '{}' was added at index '{}'", i, j);
                    break;
                }
            }
        }
    }

	InputManager& InputManager::GetInstance()
	{
		static InputManager instance{};
		return instance;
	}

	void InputManager::CreateLuaInputBindings(sol::state& lua, Registry& registry)
	{
        RegisterLuaKeyNames(lua);
        RegisterLuaMouseButtonNames(lua);
        RegisterLuaGamepadButtonNames(lua);

        auto& inputManager = GetInstance();
        auto& camera = registry.GetContext<std::shared_ptr<Camera2D>>();

        auto& keyboard = inputManager.GetKeyboard();
        lua.new_usertype<Keyboard>(
            "Keyboard",
            sol::no_constructor,
            "justPressed", [&](int key) { return keyboard.IsKeyJustPressed(key); },
            "justReleased", [&](int key) { return keyboard.IsKeyJustReleased(key); },
            "pressed", [&](int key) { return keyboard.IsKeyPressed(key); },
            "pressedKeys", [&]()
            {
                std::vector<int> keys;
                for (const auto& [key, button] : keyboard.GetButtonMap())
                {
                    if (button.IsPressed)
                        keys.push_back(key);
                }
                return keys;
            }
        );

// NOTE: In order for this to work in the editor, we need to take into account the imgui window position, to get the world position.
#ifdef IN_FEATHER_EDITOR
        auto& mouse = inputManager.GetMouse();
        lua.new_usertype<Mouse>(
            "Mouse",
            sol::no_constructor,
            "justPressed", [&](int btn) { return mouse.IsButtonJustPressed(btn); },
            "justReleased", [&](int btn) { return mouse.IsButtonJustReleased(btn); },
            "pressed", [&](int btn) { return mouse.IsButtonPressed(btn); },
            "screenPosition", [&]()
            {
                auto [x, y] = mouse.GetMouseScreenPosition();
                return glm::vec2{ x, y };
            },
            "worldPosition", [&]()
            {
                auto [x, y] = mouse.GetMouseScreenPosition();
                return camera->ScreenCoordsToWorld(glm::vec2{ x, y });
            },
            "wheelX", [&]() { return mouse.GetMouseWheelX(); },
            "wheelY", [&]() { return mouse.GetMouseWheelY(); }
        );
#endif

        lua.new_usertype<Gamepad>(
            "Gamepad",
            sol::no_constructor,
            "anyConnected", [&]
            {
                return inputManager.GamepadConnected();
            },
            "connected", [&](int index)
            {
                return inputManager.GamepadConnected(index);
            },
            "justPressed", [&](int index, int btn)
            {
                auto gamepad = inputManager.GetController(index);
                if (!gamepad)
                {
                    F_ERROR("Invalid gamepad index '{}' provided or gamepad is not plugged in!", index);
                    return false;
                }
                return gamepad->IsButtonJustPressed(btn);
            },
            "justReleased", [&](int index, int btn)
            {
                auto gamepad = inputManager.GetController(index);
                if (!gamepad)
                {
                    F_ERROR("Invalid gamepad index '{}' provided or gamepad is not plugged in!", index);
                    return false;
                }
                return gamepad->IsButtonJustReleased(btn);
            },
            "pressed", [&](int index, int btn)
            {
                auto gamepad = inputManager.GetController(index);
                if (!gamepad)
                {
                    F_ERROR("Invalid gamepad index '{}' provided or gamepad is not plugged in!", index);
                    return false;
                }
                return gamepad->IsButtonPressed(btn);
            },
            "getAxisPosition", [&](int index, int axis)
            {
                auto gamepad = inputManager.GetController(index);
                if (!gamepad)
                {
                    F_ERROR("Invalid gamepad index '{}' provided or gamepad is not plugged in!", index);
                    return Sint16{ 0 };
                }
                return gamepad->GetAxisPosition(axis);
            },
            "getHatValue", [&](int index)
            {
                auto gamepad = inputManager.GetController(index);
                if (!gamepad)
                {
                    F_ERROR("Invalid gamepad index '{}' provided or gamepad is not plugged in!", index);
                    return Uint8{ 0 };
                }
                return gamepad->GetJoystickHatValue();
            },
            "rumble", [&](int index, Uint16 lowFrequency, Uint16 highFrequency, Uint32 durationMs)
            {
                auto gamepad = inputManager.GetController(index);
                if (!gamepad)
                {
                    F_ERROR("Invalid gamepad index '{}' provided or gamepad is not plugged in!", index);
                    return;
                }

                gamepad->RumbleController(lowFrequency, highFrequency, durationMs);
            }
        );
	}

    void InputManager::UpdateInputs()
    {
        m_Keyboard->Update();
        m_Mouse->Update();
        UpdateGamepads();
    }

    bool InputManager::GamepadConnected() const
    {
        return std::ranges::any_of(m_GameControllers, [](const auto& pair) { return pair.second != nullptr; });
    }

    bool InputManager::GamepadConnected(int location) const
    {
        auto gamepadItr = m_GameControllers.find(location);
        return gamepadItr != m_GameControllers.end() && gamepadItr->second != nullptr;
    }

    std::shared_ptr<Gamepad> InputManager::GetController(int index)
    {
        auto gamepadItr = m_GameControllers.find(index);
        if (gamepadItr == m_GameControllers.end())
        {
            F_ERROR("Failed to get gamepad at index '{0}' - Does not exist!", index);
            return nullptr;
        }

        return gamepadItr->second;
    }

    int InputManager::AddGamepad(Sint32 gamepadID)
    {
        // Trying to add a controller with the same id
        if (std::ranges::any_of(m_GameControllers, [&gamepadID](const auto& pair) { return pair.second->CheckJoystickID(gamepadID); }))
        {
            F_WARN("Trying to add a controller that is already mapped. Gamepad ID: {}", gamepadID);
            return -1;
        }

        if (m_GameControllers.size() >= MAX_CONTROLLERS)
        {
            F_WARN("Trying to add too many controllers! Max controllers allowed = {}", MAX_CONTROLLERS);
            return -1;
        }

        std::shared_ptr<Gamepad> gamepad{ nullptr };
        try
        {
            gamepad = std::make_shared<Gamepad>(std::move(MakeSharedFromSDLType<Controller>(SDL_GameControllerOpen(gamepadID))));
        }
        catch (...)
        {
            std::string error{ SDL_GetError() };
            F_ERROR("Failed to open gamepad device: {}", error);
            return -1;
        }

        for (int i = 1; i <= MAX_CONTROLLERS; i++)
        {
            if (m_GameControllers.contains(i))
                continue;

            m_GameControllers.emplace(i, std::move(gamepad));
            F_TRACE("Gamepad '{}' added at index {}", gamepadID, i);
            return i;
        }

        F_ASSERT(false && "Failed to add the new controller!");
        F_ERROR("Failed to add the new controller!");
        return -1;
    }

    int InputManager::RemoveGamepad(Sint32 gamepadID)
    {
        auto gamepadItr = std::ranges::find_if(m_GameControllers, [&](const auto& gamepad) { return gamepad.second->CheckJoystickID(gamepadID); });

        int index{ -1 };
        if (gamepadItr == m_GameControllers.end())
        {
            F_ASSERT(false && "Failed to remove Gamepad must not have been mapped correctly");
            F_ERROR("Failed to remove Gamepad ID '{}' must not have been mapped correctly", gamepadID);
            return index;
        }

        index = gamepadItr->first;
        m_GameControllers.erase(gamepadItr);
        F_TRACE("Gamepad ID '{}' at index '{}' was removed", gamepadID, index);
        return index;
    }

    void InputManager::GamepadButtonPressed(const SDL_Event& event)
    {
        for (const auto& [index, gamepad] : m_GameControllers)
        {
            if (gamepad && gamepad->CheckJoystickID(event.jdevice.which))
            {
                gamepad->OnButtonPressed(event.cbutton.button);
                break;
            }
        }
    }

    void InputManager::GamepadButtonReleased(const SDL_Event& event)
    {
        for (const auto& [index, gamepad] : m_GameControllers)
        {
            if (gamepad && gamepad->CheckJoystickID(event.jdevice.which))
            {
                gamepad->OnButtonReleased(event.cbutton.button);
                break;
            }
        }
    }

    void InputManager::GamepadAxisValues(const SDL_Event& event)
    {
        for (const auto& [index, gamepad] : m_GameControllers)
        {
            if (gamepad && gamepad->CheckJoystickID(event.jdevice.which))
            {
                gamepad->SetAxisPositionValue(event.jaxis.axis, event.jaxis.value);
                break;
            }
        }
    }

    void InputManager::GamepadHatValues(const SDL_Event& event)
    {
        for (const auto& [index, gamepad] : m_GameControllers)
        {
            if (gamepad && gamepad->CheckJoystickID(event.jdevice.which))
            {
                gamepad->SetJoystickHatValue(event.jhat.value);
                break;
            }
        }
    }

    void InputManager::UpdateGamepads()
    {
        for (const auto& [index, gamepad] : m_GameControllers)
        {
            if (gamepad)
                gamepad->Update();
        }
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

        // Register Punctuation Keys
        lua.set("KEY_COLON", F_KEY_COLON);
        lua.set("KEY_SEMICOLON", F_KEY_SEMICOLON);
        lua.set("KEY_QUOTE", F_KEY_QUOTE);
        lua.set("KEY_BACKQUOTE", F_KEY_BACKQUOTE);
        lua.set("KEY_CARET", F_KEY_CARET);
        lua.set("KEY_UNDERSCORE", F_KEY_UNDERSCORE);
        lua.set("KEY_RIGHTBRACKET", F_KEY_RIGHTBRACKET);
        lua.set("KEY_LEFTBRACKET", F_KEY_LEFTBRACKET);
        lua.set("KEY_SLASH", F_KEY_SLASH);
        lua.set("KEY_ASTERISK", F_KEY_ASTERISK);
        lua.set("KEY_LEFTPAREN", F_KEY_LEFTPAREN);
        lua.set("KEY_RIGHTPAREN", F_KEY_RIGHTPAREN);
        lua.set("KEY_QUESTION", F_KEY_QUESTION);
        lua.set("KEY_AMPERSAND", F_KEY_AMPERSAND);
        lua.set("KEY_DOLLAR", F_KEY_DOLLAR);
        lua.set("KEY_EXCLAIM", F_KEY_EXCLAIM);
        lua.set("KEY_BACKSLASH", F_KEY_BACKSLASH);

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

        lua.set("KP_KEY_DIVIDE", F_KEY_KP_DIVIDE);
        lua.set("KP_KEY_MULTIPLY", F_KEY_KP_MULTIPLY);
        lua.set("KP_KEY_MINUS", F_KEY_KP_MINUS);
        lua.set("KP_KEY_PLUS", F_KEY_KP_PLUS);
        lua.set("KP_KEY_ENTER", F_KEY_KP_ENTER);
        lua.set("KP_KEY_PERIOD", F_KEY_KP_PERIOD);
        lua.set("KEY_NUM_LOCK", F_KEY_NUMLOCK);

        // Windows specific keys
#ifdef _WIN32
        lua.set("KEY_LWIN", F_KEY_LWIN);
        lua.set("KEY_RWIN", F_KEY_RWIN);
#else 
        lua.set("KEY_LGUI", F_KEY_LGUI);
        lua.set("KEY_RGUI", F_KEY_RGUI);
#endif
	}

    void InputManager::RegisterLuaMouseButtonNames(sol::state& lua)
    {
        lua.set("LEFT_BUTTON", F_MOUSE_LEFT);
        lua.set("MIDDLE_BUTTON", F_MOUSE_MIDDLE);
        lua.set("RIGHT_BUTTON", F_MOUSE_RIGHT);
    }

    void InputManager::RegisterLuaGamepadButtonNames(sol::state& lua)
    {
        lua.set("GP_BUTTON_A", F_GP_A);
        lua.set("GP_BUTTON_B", F_GP_B);
        lua.set("GP_BUTTON_X", F_GP_X);
        lua.set("GP_BUTTON_Y", F_GP_Y);

        lua.set("GP_BUTTON_BACK", F_GP_BACK);
        lua.set("GP_BUTTON_GUIDE", F_GP_GUIDE);
        lua.set("GP_BUTTON_START", F_GP_START);

        lua.set("GP_LSTICK", F_GP_LSTICK);
        lua.set("GP_RSTICK", F_GP_RSTICK);

        lua.set("GP_LSHOULDER", F_GP_LSHOULDER);
        lua.set("GP_RSHOULDER", F_GP_RSHOULDER);

        lua.set("GP_DPAD_UP", F_GP_DPAD_UP);
        lua.set("GP_DPAD_DOWN", F_GP_DPAD_DOWN);
        lua.set("GP_DPAD_LEFT", F_GP_DPAD_LEFT);
        lua.set("GP_DPAD_RIGHT", F_GP_DPAD_RIGHT);

        lua.set("GP_AXIS_X1", 0); lua.set("GP_AXIS_Y1", 1);
        lua.set("GP_AXIS_X2", 2); lua.set("GP_AXIS_Y2", 3);

        // NOTE: bottom triggers
        lua.set("GP_AXIS_Z1", 4); lua.set("GP_AXIS_Z2", 5);
    }

}
