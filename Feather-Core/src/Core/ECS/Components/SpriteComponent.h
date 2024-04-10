#pragma once

#include <sol/sol.hpp>

#include "Renderer/Essentials/Vertex.h"
#include "Core/ECS/Registry.h"

namespace Feather {

	struct UVs
	{
		float u{ 0.0f }, v{ 0.0f }, uv_width{ 0.0f }, uv_height{ 0.0f };
	};

	struct SpriteComponent
	{
		float width{ 0.0f }, height{ 0.0f };
		UVs uvs{ .u = 0.0f, .v = 0.0f, .uv_width = 0.0f, .uv_height = 0.0f };

		Feather::Color color{ .r = 255, .g = 255, .b = 255, .a = 255 };
		int start_x{ 0 }, start_y{ 0 }, layer{ 0 };

		bool isHidden{ false };
		std::string texture_name{ "" };

		void generate_uvs(int textureWidth, int textureHeight);
		[[nodiscard]] std::string to_string() const;

		static void CreateSpriteLuaBind(sol::state& lua, Feather::Registry& registry);
	};

}
