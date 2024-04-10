#include "TextComponent.h"

#include <entt.hpp>

std::string Feather::TextComponent::to_string()
{
	std::stringstream ss;
	ss << "==== Text Component ==== \n" << std::boolalpha <<
		  "Text: " << textStr << "\n" <<
		  "Font Name: " << fontName << "\n" <<
		  "Padding: " << padding << "\n" <<
		  "Wrap: " << wrap << "\n" <<
		  "IsHidden: " << isHidden << "\n" <<
		  "Color: \n\t" <<
		  "Red: " << color.r << "\n\t" <<
		  "Green: " << color.g << "\n\t" <<
		  "Blue: " << color.b << "\n\t" <<
		  "Alpha: " << color.a << "\n";

	return ss.str();
}

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
			"isHidden", &TextComponent::isHidden,
			"to_string", &TextComponent::to_string
		);
	}

}
