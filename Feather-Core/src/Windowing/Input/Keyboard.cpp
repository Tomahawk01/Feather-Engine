#include "Keyboard.h"

#include "Logger/Logger.h"

namespace Feather {

    Keyboard::Keyboard()
        : m_Buttons{
        {F_KEY_BACKSPACE, Button{}}, {F_KEY_TAB, Button{}}, {F_KEY_CLEAR, Button{}},
        {F_KEY_RETURN, Button{}}, {F_KEY_PAUSE, Button{}}, {F_KEY_ESCAPE, Button{}}, {F_KEY_SPACE, Button{}},
        {F_KEY_EXCLAIM, Button{}}, {F_KEY_QUOTEDBL, Button{}}, {F_KEY_HASH, Button{}}, {F_KEY_DOLLAR, Button{}},
        {F_KEY_AMPERSAND, Button{}}, {F_KEY_QUOTE, Button{}}, {F_KEY_LEFTPAREN, Button{}}, {F_KEY_RIGHTPAREN, Button{}},
        {F_KEY_ASTERISK, Button{}}, {F_KEY_PLUS, Button{}}, {F_KEY_COMMA, Button{}}, {F_KEY_PERIOD, Button{}},
        {F_KEY_SLASH, Button{}}, {F_KEY_0, Button{}}, {F_KEY_2, Button{}}, {F_KEY_3, Button{}},
        {F_KEY_4, Button{}}, {F_KEY_5, Button{}}, {F_KEY_6, Button{}}, {F_KEY_7, Button{}},
        {F_KEY_8, Button{}}, {F_KEY_9, Button{}}, {F_KEY_COLON, Button{}}, {F_KEY_SEMICOLON, Button{}},
        {F_KEY_LESS, Button{}}, {F_KEY_EQUALS, Button{}}, {F_KEY_GREATER, Button{}}, {F_KEY_QUESTION, Button{}},
        {F_KEY_AT, Button{}}, {F_KEY_LEFTBRACKET, Button{}}, {F_KEY_BACKSLASH, Button{}}, {F_KEY_RIGHTBRACKET, Button{}},
        {F_KEY_CARET, Button{}}, {F_KEY_UNDERSCORE, Button{}}, {F_KEY_A, Button{}}, {F_KEY_B, Button{}},
        {F_KEY_C, Button{}}, {F_KEY_D, Button{}}, {F_KEY_E, Button{}}, {F_KEY_F, Button{}},
        {F_KEY_G, Button{}}, {F_KEY_H, Button{}}, {F_KEY_I, Button{}}, {F_KEY_J, Button{}},
        {F_KEY_K, Button{}}, {F_KEY_L, Button{}}, {F_KEY_M, Button{}}, {F_KEY_N, Button{}},
        {F_KEY_O, Button{}}, {F_KEY_P, Button{}}, {F_KEY_Q, Button{}}, {F_KEY_R, Button{}},
        {F_KEY_S, Button{}}, {F_KEY_T, Button{}}, {F_KEY_U, Button{}}, {F_KEY_V, Button{}},
        {F_KEY_W, Button{}}, {F_KEY_X, Button{}}, {F_KEY_Y, Button{}}, {F_KEY_Z, Button{}},
        {F_KEY_DELETE, Button{}}, {F_KEY_CAPSLOCK, Button{}}, {F_KEY_F1, Button{}}, {F_KEY_F2, Button{}},
        {F_KEY_F3, Button{}}, {F_KEY_F4, Button{}}, {F_KEY_F5, Button{}}, {F_KEY_F6, Button{}},
        {F_KEY_F7, Button{}}, {F_KEY_F8, Button{}}, {F_KEY_F9, Button{}}, {F_KEY_F10, Button{}},
        {F_KEY_F11, Button{}}, {F_KEY_F12, Button{}}, {F_KEY_SCROLLOCK, Button{}}, {F_KEY_INSERT, Button{}},
        {F_KEY_HOME, Button{}}, {F_KEY_PAGEUP, Button{}}, {F_KEY_PAGEDOWN, Button{}}, {F_KEY_END, Button{}},
        {F_KEY_RIGHT, Button{}}, {F_KEY_LEFT, Button{}}, {F_KEY_DOWN, Button{}}, {F_KEY_UP, Button{}},
        {F_KEY_NUMLOCK, Button{}}, {F_KEY_KP_DIVIDE, Button{}}, {F_KEY_KP_MULTIPLY, Button{}}, {F_KEY_KP_MINUS, Button{}},
        {F_KEY_KP_PLUS, Button{}}, {F_KEY_KP_ENTER, Button{}}, {F_KEY_KP1, Button{}}, {F_KEY_KP2, Button{}},
        {F_KEY_KP3, Button{}}, {F_KEY_KP4, Button{}}, {F_KEY_KP5, Button{}}, {F_KEY_KP6, Button{}},
        {F_KEY_KP7, Button{}}, {F_KEY_KP8, Button{}}, {F_KEY_KP9, Button{}}, {F_KEY_KP0, Button{}},
        {F_KEY_KP_PERIOD, Button{}}, {F_KEY_LCTRL, Button{}}, {F_KEY_LSHIFT, Button{}},
        {F_KEY_LALT, Button{}}, {F_KEY_RCTRL, Button{}}, {F_KEY_RSHIFT, Button{}}, {F_KEY_RALT, Button{}}
        }
    {}

    void Keyboard::Update()
    {
        for (auto& [key, button] : m_Buttons)
            button.Reset();
    }

    void Keyboard::OnKeyPressed(int key)
    {
        if (key == KEY_UNKNOWN)
        {
            F_ERROR("Key '{0}' is unknown!", key);
            return;
        }

        auto keyItr = m_Buttons.find(key);
        if (keyItr == m_Buttons.end())
        {
            F_ERROR("Key '{0}' does not exist!", key);
            return;
        }
        keyItr->second.Update(true);
    }

    void Keyboard::OnKeyReleased(int key)
    {
        if (key == KEY_UNKNOWN)
        {
            F_ERROR("Key '{0}' is unknown!", key);
            return;
        }

        auto keyItr = m_Buttons.find(key);
        if (keyItr == m_Buttons.end())
        {
            F_ERROR("Key '{0}' does not exist!", key);
            return;
        }
        keyItr->second.Update(false);
    }

    const bool Keyboard::IsKeyPressed(int key) const
    {
        if (key == KEY_UNKNOWN)
        {
            F_ERROR("Key '{0}' is unknown!", key);
            return false;
        }

        auto keyItr = m_Buttons.find(key);
        if (keyItr == m_Buttons.end())
        {
            F_ERROR("Key '{0}' does not exist!", key);
            return false;
        }
        return keyItr->second.IsPressed;
    }

    const bool Keyboard::IsKeyJustPressed(int key) const
    {
        if (key == KEY_UNKNOWN)
        {
            F_ERROR("Key '{0}' is unknown!", key);
            return false;
        }

        auto keyItr = m_Buttons.find(key);
        if (keyItr == m_Buttons.end())
        {
            F_ERROR("Key '{0}' does not exist!", key);
            return false;
        }
        return keyItr->second.JustPressed;
    }

    const bool Keyboard::IsKeyJustReleased(int key) const
    {
        if (key == KEY_UNKNOWN)
        {
            F_ERROR("Key '{0}' is unknown!", key);
            return false;
        }

        auto keyItr = m_Buttons.find(key);
        if (keyItr == m_Buttons.end())
        {
            F_ERROR("Key '{0}' does not exist!", key);
            return false;
        }
        return keyItr->second.JustReleased;
    }

}
