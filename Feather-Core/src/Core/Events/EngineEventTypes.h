#pragma once

#include "Physics/UserData.h"

#include <sol/sol.hpp>

namespace Feather {

    struct ContactEvent
    {
        ObjectData objectA{};
        ObjectData objectB{};
    };

    enum class EKeyEventType
    {
        Pressed,
        Released,
        NoType
    };

    struct KeyEvent
    {
        int key{ -1 };
        EKeyEventType type{ EKeyEventType::NoType };
    };

    enum class GamepadConnectType
    {
        Connected,
        Disconnected,
        NotConnected
    };

    struct GamepadConnectEvent
    {
        GamepadConnectType connectType{ GamepadConnectType::NotConnected };
        int index{ 1 };
    };

    struct LuaEvent
    {
        sol::object data{ sol::lua_nil };
    };

    struct LuaEventBinder
    {
        static void CreateLuaEventBindings(sol::state& lua);
    };

}
