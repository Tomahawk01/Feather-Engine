#include "EditorCoreLuaWrappers.h"

#include "Core/Scripting/InputManager.h"
#include "Core/ECS/Registry.h"
#include "Renderer/Core/Camera2D.h"
#include "Editor/Utilities/EditorUtilities.h"
#include "Logger/Logger.h"

#include <glm/glm.hpp>
#include <imgui.h>
#include <cmath>

namespace Feather {

	void LuaCoreBinder::CreateLuaBind(sol::state& lua, Registry& registry)
	{
		auto& mouse = INPUT_MANAGER().GetMouse();
		auto& camera = registry.GetContext<std::shared_ptr<Camera2D>>();

        lua.new_usertype<Mouse>(
            "Mouse",
            sol::no_constructor,
            "just_pressed",
            [&](int btn) { return mouse.IsButtonJustPressed(btn); },
            "just_released",
            [&](int btn) { return mouse.IsButtonJustReleased(btn); },
            "pressed",
            [&](int btn) { return mouse.IsButtonPressed(btn); },
            "screen_position",
            [&]()
            {
                const auto& mouseInfo = registry.GetContext<std::shared_ptr<MouseGuiInfo>>();
                // If the mouse info is invalid, return the mouse screen position from SDL.
                // This does not take into account ImGui::Docking, windowsize, relative position, etc.
                if (!mouseInfo)
                {
                    auto [x, y] = mouse.GetMouseScreenPosition();
                    return glm::vec2{ x, y };
                }

                return mouseInfo->position;
            },
            "world_position",
            [&]()
            {
                const auto& mouseInfo = registry.GetContext<std::shared_ptr<MouseGuiInfo>>();
                // If the mouse info is invalid, return the mouse screen position from SDL.
                // This does not take into account ImGui::Docking, windowsize, relative position, etc.
                if (!mouseInfo)
                {
                    auto [x, y] = mouse.GetMouseScreenPosition();
                    return camera->ScreenCoordsToWorld(glm::vec2{ x, y });
                }

                float widthRatio = mouseInfo->windowSize.x / static_cast<float>(camera->GetWidth());
                float heightRatio = mouseInfo->windowSize.y / static_cast<float>(camera->GetHeight());
                F_ASSERT(widthRatio > 0 && heightRatio > 0 && "Ensure the width and height ratios are above zero!");
                int x = mouseInfo->position.x / widthRatio;
                int y = mouseInfo->position.y / heightRatio;

                return camera->ScreenCoordsToWorld(glm::vec2{ x, y });
            },
            "wheel_x",
            [&]() { return mouse.GetMouseWheelX(); },
            "wheel_y",
            [&]() { return mouse.GetMouseWheelY(); }
        );

        // TODO: Add more editor specific core bindings here
	}

}
