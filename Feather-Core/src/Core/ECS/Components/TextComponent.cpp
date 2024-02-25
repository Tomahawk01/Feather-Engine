#include "TextComponent.h"

#include <entt.hpp>

namespace Feather {

	void TextComponent::CreateLuaTextBindings(sol::state& lua)
	{
		lua.new_usertype<TextComponent>(
			"TextComponent",
			"type_id", entt::type_hash<TextComponent>::value,
			sol::call_constructor,
			sol::factories(
				[](const std::string& fontName, const std::string textStr, Color color, int padding, float wrap)
				{
					return TextComponent{
						.fontName = fontName,
						.textStr = textStr,
						.padding = padding,
						.wrap = wrap,
						.color = color
					};
				}
			),
			"textStr", &TextComponent::textStr,
			"fontName", &TextComponent::fontName,
			"padding", &TextComponent::padding,
			"wrap", &TextComponent::wrap,
			"color", &TextComponent::color,
			"isHidden", &TextComponent::isHidden
		);
	}

}
