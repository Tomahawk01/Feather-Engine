#pragma once

#include <sol/sol.hpp>

namespace Feather {

    enum class UIObjectType
    {
        PANEL,
        SELECTOR,
        PANEL_LAYOUT,
        TEXTBOX,

        NO_TYPE
    };

    struct UIComponent
    {
        UIObjectType type{ UIObjectType::NO_TYPE };

        static void CreateLuaBind(sol::state& lua);
    };

}
