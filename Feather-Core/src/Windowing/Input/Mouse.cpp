#include "Mouse.h"

#include <SDL.h>

#include "Logger/Logger.h"

namespace Feather {

    void Mouse::Update()
    {
        for (auto& [btn, button] : m_Buttons)
            button.Reset();
        
        m_WheelX = 0;
        m_WheelY = 0;
        m_MouseMoving = false;
    }

    void Mouse::OnButtonPressed(int btn)
    {
        if (btn == MOUSE_UNKNOWN)
        {
            F_ERROR("Mouse button '{0}' is unknown!", btn);
            return;
        }

        auto btnItr = m_Buttons.find(btn);
        if (btnItr == m_Buttons.end())
        {
            F_ERROR("Mouse button '{0}' does not exist!", btn);
            return;
        }

        btnItr->second.Update(true);
    }

    void Mouse::OnButtonReleased(int btn)
    {
        if (btn == MOUSE_UNKNOWN)
        {
            F_ERROR("Mouse button '{0}' is unknown!", btn);
            return;
        }

        auto btnItr = m_Buttons.find(btn);
        if (btnItr == m_Buttons.end())
        {
            F_ERROR("Mouse button '{0}' does not exist!", btn);
            return;
        }

        btnItr->second.Update(false);
    }

    const bool Mouse::IsButtonPressed(int btn) const
    {
        if (btn == MOUSE_UNKNOWN)
        {
            F_ERROR("Mouse button '{0}' is unknown!", btn);
            return false;
        }

        auto btnItr = m_Buttons.find(btn);
        if (btnItr == m_Buttons.end())
        {
            F_ERROR("Mouse button '{0}' does not exist!", btn);
            return false;
        }

        return btnItr->second.IsPressed;
    }

    const bool Mouse::IsButtonJustPressed(int btn) const
    {
        if (btn == MOUSE_UNKNOWN)
        {
            F_ERROR("Mouse button '{0}' is unknown!", btn);
            return false;
        }

        auto btnItr = m_Buttons.find(btn);
        if (btnItr == m_Buttons.end())
        {
            F_ERROR("Mouse button '{0}' does not exist!", btn);
            return false;
        }

        return btnItr->second.JustPressed;
    }

    const bool Mouse::IsButtonJustReleased(int btn) const
    {
        if (btn == MOUSE_UNKNOWN)
        {
            F_ERROR("Mouse button '{0}' is unknown!", btn);
            return false;
        }

        auto btnItr = m_Buttons.find(btn);
        if (btnItr == m_Buttons.end())
        {
            F_ERROR("Mouse button '{0}' does not exist!", btn);
            return false;
        }

        return btnItr->second.JustReleased;
    }

    const std::tuple<int, int> Mouse::GetMouseScreenPosition()
    {
        SDL_GetMouseState(&m_X, &m_Y);
        return std::make_tuple(m_X, m_Y);
    }

}
