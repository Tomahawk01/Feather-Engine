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
				},
				[](const std::string& fontName, const std::string textStr)
				{
					return TextComponent{
						.fontName = fontName,
						.textStr = textStr
					};
				}
			),
			"textStr", &TextComponent::textStr,
			"fontName", &TextComponent::fontName,
			"padding", &TextComponent::padding,
			"wrap", &TextComponent::wrap,
			"color", &TextComponent::color,
			"setWrap", // Should be used instead of direct member variables
			[](TextComponent& text, const float wrap)
			{
				text.wrap = wrap;
				text.isDirty = true;
			},
			"setText", // Should be used instead of direct member variables
			[](TextComponent& text, const std::string& sText)
			{
				text.textStr = sText;
				text.isDirty = true;
			},
			"setFont", // Should be used instead of direct member variables
			[](TextComponent& text, const std::string& font)
			{
				text.fontName = font;
				text.isDirty = true;
			},
			"setPadding", // Should be used instead of direct member variables
			[](TextComponent& text, const int padding)
			{
				text.padding = padding;
				text.isDirty = true;
			},
			"isHidden", &TextComponent::isHidden,
			"toString", &TextComponent::to_string
		);
	}

}
