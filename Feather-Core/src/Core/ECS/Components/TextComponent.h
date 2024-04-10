#pragma once

#include "Renderer/Essentials/Vertex.h"

#include <sol/sol.hpp>

namespace Feather {

	struct TextComponent
	{
		std::string fontName{ "testFont" }, textStr{ "" };
		int padding{ 0 };
		float wrap{ -1.0f };
		Color color{ 255,255,255,255 };
		bool isHidden{ false };

		[[nodiscard]] std::string to_string();

		static void CreateLuaTextBindings(sol::state& lua);
	};

}
