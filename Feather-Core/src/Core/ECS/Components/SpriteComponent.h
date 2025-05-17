#pragma once

#include <sol/sol.hpp>

#include "Renderer/Essentials/Vertex.h"
#include "Core/ECS/Registry.h"

namespace Feather {

	struct UVs
	{
		float u{ 0.0f };
		float v{ 0.0f };
		float uv_width{ 0.0f };
		float uv_height{ 0.0f };
	};

	struct SpriteComponent
	{
		std::string textureName{ "" };
		float width{ 16.0f };
		float height{ 16.0f };
		UVs uvs{ .u = 0.0f, .v = 0.0f, .uv_width = 0.0f, .uv_height = 0.0f };

		Feather::Color color{ .r = 255, .g = 255, .b = 255, .a = 255 };
		int start_x{ 0 };
		int start_y{ 0 };
		int layer{ 0 };

		bool isHidden{ false };
		bool isIsometric{ false };
		/* Iso cell is needed to sort when rendering */
		int isoCellX{ 0 };
		/* Iso cell is needed to sort when rendering */
		int isoCellY{ 0 };

		// void generate_uvs(int textureWidth, int textureHeight);
		[[nodiscard]] std::string to_string() const;

		static void CreateSpriteLuaBind(sol::state& lua);
	};

}
