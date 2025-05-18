#include "SpriteComponent.h"

#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/CoreUtils/CoreUtilities.h"

std::string Feather::SpriteComponent::to_string() const
{
	std::stringstream ss;
	ss << "==== Sprite Component ==== \n" << std::boolalpha <<
		  "Texture Name: " << textureName << "\n" <<
		  "Width: " << width << "\n" <<
		  "Height: " << height << "\n" <<
		  "StartX: " << start_x << "\n" <<
		  "StartY: " << start_y << "\n" <<
		  "Layer: " << layer << "\n" <<
		  "UVs: \n\t" <<
		  "U: " << uvs.u << "\n\t" <<
		  "V: " << uvs.v << "\n\t" <<
		  "UvWidth: " << uvs.uv_width << "\n\t" <<
		  "UvHeight: " << uvs.uv_width << "\n" <<
		  "Color: \n\t" <<
		  "Red: " << color.r << "\n\t" <<
		  "Green: " << color.g << "\n\t" <<
		  "Blue: " << color.b << "\n\t" <<
		  "Alpha: " << color.a << "\n" <<
		  "IsHidden: " << isHidden << "\n";

	return ss.str();
}

void Feather::SpriteComponent::CreateSpriteLuaBind(sol::state& lua)
{
	auto& mainRegistry = MAIN_REGISTRY();
	auto& assetManager = mainRegistry.GetAssetManager();

	lua.new_usertype<Color>(
		"Color",
		sol::call_constructor,
		sol::factories(
			[](GLubyte r, GLubyte g, GLubyte b, GLubyte a) { return Color{ .r = r, .g = g, .b = b, .a = a }; }
		),
		"r", &Color::r,
		"g", &Color::g,
		"b", &Color::b,
		"a", &Color::a
	);

	lua.new_usertype<UVs>(
		"UVs",
		sol::call_constructor,
		sol::factories(
			[](float u, float v) { return UVs{ .u = u, .v = v }; }
		),
		"u", &UVs::u,
		"v", &UVs::v,
		"uvWidth", &UVs::uv_width,
		"uvHeight", &UVs::uv_height
	);

	lua.new_usertype<SpriteComponent>(
		"Sprite",
		"type_id", &entt::type_hash<SpriteComponent>::value,
		sol::call_constructor,
		sol::factories(
			[](const std::string& textureName, float width, float height, int start_x, int start_y, int layer)
			{
				return SpriteComponent{
					.textureName = textureName,
					.width = width,
					.height = height,
					.uvs = UVs{},
					.color = Feather::Color{255, 255, 255, 255},
					.start_x = start_x,
					.start_y = start_y,
					.layer = layer
				};
			}
		),
		"texture_name", &SpriteComponent::textureName,
		"width", &SpriteComponent::width,
		"height", &SpriteComponent::height,
		"startX", &SpriteComponent::start_x,
		"startY", &SpriteComponent::start_y,
		"layer", &SpriteComponent::layer,
		"hidden", &SpriteComponent::isHidden,
		"uvs", &SpriteComponent::uvs,
		"color", &SpriteComponent::color,
		"generateUVs", [&](SpriteComponent& sprite)
		{
			auto pTexture = assetManager.GetTexture(sprite.textureName);

			if (!pTexture)
			{
				F_ERROR("Failed to generate uvs - texture '{0}' does not exists or invalid!", sprite.textureName);
				return;
			}

			GenerateUVs(sprite, pTexture->GetWidth(), pTexture->GetHeight());
		},
		"inspectUVs", [](SpriteComponent& sprite) {
			sprite.uvs.u = sprite.start_x * sprite.uvs.uv_width;
			sprite.uvs.v = sprite.start_y * sprite.uvs.uv_height;
		},
		"inspectX", [](SpriteComponent& sprite) {
			sprite.uvs.u = sprite.start_x * sprite.uvs.uv_width;
		},
		"inspectY", [](SpriteComponent& sprite) {
			sprite.uvs.v = sprite.start_y * sprite.uvs.uv_height;
		}
	);
}
