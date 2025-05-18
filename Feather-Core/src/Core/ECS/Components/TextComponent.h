#pragma once

#include "Renderer/Essentials/Vertex.h"

#include <sol/sol.hpp>

namespace Feather {

	struct TextComponent
	{
		std::string fontName{ "testFont" };
		std::string textStr{ "" };
		int padding{ 0 };
		float wrap{ -1.0f };
		float textBoxWidth{ -1.0f };
		float textBoxHeight{ -1.0f };
		Color color{ 255, 255, 255, 255 };
		bool isHidden{ false };
		bool isDirty{ false };

		[[nodiscard]] std::string to_string();

		static void CreateLuaTextBindings(sol::state& lua);
	};

}
