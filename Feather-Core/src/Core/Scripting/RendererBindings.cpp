#include "RendererBindings.h"

#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/MainRegistry.h"
#include "Renderer/Essentials/Primitives.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Core/Renderer.h"

namespace Feather {

	void RendererBinder::CreateRenderingBind(sol::state& lua, Registry& registry)
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		// Primitives bind
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

		lua.new_usertype<Circle>(
			"Circle",
			sol::call_constructor,
			sol::factories(
				[](const glm::vec2& position, float lineThickness, float radius, const Color& color)
				{
					return Circle{ .position = position, .lineThickness = lineThickness, .radius = radius, .color = color };
				}
			),
			"position", &Circle::position,
			"lineThickness", &Circle::lineThickness,
			"radius", &Circle::radius,
			"color", &Circle::color
		);

		lua.new_usertype<Text>(
			"Text",
			sol::call_constructor,
			sol::factories(
				[&](const glm::vec2& position, const std::string& textStr, const std::string& fontName, float wrap, const Color& color)
				{
					auto font = assetManager.GetFont(fontName);
					if (!font)
					{
						F_ERROR("Failed to get font '{0}': Does not exist in asset manager!", fontName);
						return Text{};
					}

					return Text{
						.position = position,
						.textStr = textStr,
						.wrap = wrap,
						.pFont = font,
						.color = color
					};
				}
			),
			"position", &Text::position,
			"textStr", &Text::textStr,
			"wrap", &Text::wrap,
			"color", &Text::color
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

		lua.set_function(
			"DrawCircle", sol::overload(
				[&](const Circle& circle)
				{
					renderer->DrawCircle(circle);
				},
				[&](const glm::vec2& pos, float lineThickness, float radius, const Color& color)
				{
					renderer->DrawCircle(pos, radius, color, lineThickness);
				}
			)
		);

		lua.set_function(
			"DrawFilledRect", [&](const Rect& rect)
			{
				renderer->DrawFilledRect(rect);
			}
		);

		lua.set_function(
			"DrawText", [&](const Text& text)
			{
				renderer->DrawText2D(text);
			}
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
			"set_scale", [&](float scale) { camera->SetScale(scale); },
			"width", [&] { return camera->GetWidth(); },
			"height", [&] { return camera->GetHeight(); },
			"get_world_coords", [&](const glm::vec2& screenCoords) { return camera->ScreenCoordsToWorld(screenCoords); },
			"get_screen_coords", [&](const glm::vec2& worldCoords) { return camera->WorldCoordsToScreen(worldCoords); }
		);
	}

}
