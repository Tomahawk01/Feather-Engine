#include "RendererBindings.h"

#include "Logger/Logger.h"
#include "Renderer/Essentials/Primitives.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Core/Renderer.h"

namespace Feather {

	void RendererBinder::CreateRenderingBind(sol::state& lua, Registry& registry)
	{
		lua.new_usertype<Line>(
			"Line",
			sol::call_constructor,
			sol::factories(
				[](const glm::vec2& p1, const glm::vec2& p2, const Color& color)
				{
					return Line{ .p1 = p1, .p2 = p2, .color = color };
				}
			),
			"p1", &Line::p1,
			"p2", &Line::p2,
			"color", &Line::color
		);

		lua.new_usertype<Rect>(
			"Rect",
			sol::call_constructor,
			sol::factories(
				[](const glm::vec2& position, float width, float height, const Color& color)
				{
					return Rect{ .position = position, .width = width, .height = height, .color = color };
				}
			),
			"position", &Rect::position,
			"width", &Rect::width,
			"height", &Rect::height,
			"color", &Rect::color
		);

		auto& renderer = registry.GetContext<std::shared_ptr<Renderer>>();
		if (!renderer)
		{
			F_FATAL("Failed to bind Renderer to Lua! Not in the registry context!");
			return;
		}

		lua.set_function(
			"DrawLine", sol::overload(
				[&](const Line& line)
				{
					renderer->DrawLine(line);
				},
				[&](const glm::vec2& p1, const glm::vec2& p2, const Color& color)
				{
					renderer->DrawLine(p1, p2, color);
				}
			)
		);

		lua.set_function(
			"DrawRect", sol::overload(
				[&](const Rect& rect)
				{
					renderer->DrawRect(rect);
				},
				[&](const glm::vec2& position, float width, float height, const Color& color)
				{
					renderer->DrawRect(position, width, height, color);
				}
			)
		);

		auto& camera = registry.GetContext<std::shared_ptr<Camera2D>>();
		if (!camera)
		{
			F_FATAL("Failed to bind Camera to Lua! Not in the registry context!");
			return;
		}

		lua.new_usertype<Camera2D>(
			"Camera",
			sol::no_constructor,
			"get", [&] { return *camera; },
			"position", [&] { return camera->GetPosition(); },
			"scale", [&] { return camera->GetScale(); },
			"set_position", [&](const glm::vec2 newPosition) { camera->SetPosition(newPosition); },
			"set_scale", [&](float scale) { camera->SetScale(scale); }
		);
	}

}
