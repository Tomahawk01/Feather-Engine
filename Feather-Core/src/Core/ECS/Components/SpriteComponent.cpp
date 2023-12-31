#include "SpriteComponent.h"

#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"

void Feather::SpriteComponent::CreateSpriteLuaBind(sol::state& lua, Feather::Registry& registry)
{
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
		"uv_width", &UVs::uv_width,
		"uv_height", &UVs::uv_height
	);

	lua.new_usertype<SpriteComponent>(
		"Sprite",
		"type_id", &entt::type_hash<SpriteComponent>::value,
		sol::call_constructor,
		sol::factories(
			[](const std::string& textureName, float width, float height, int start_x, int start_y, int layer)
			{
				return SpriteComponent{
					.width = width,
					.height = height,
					.uvs = UVs{},
					.color = Feather::Color{255, 255, 255, 255},
					.start_x = start_x,
					.start_y = start_y,
					.layer = layer,
					.texture_name = textureName
				};
			}
		),
		"texture_name", &SpriteComponent::texture_name,
		"width", &SpriteComponent::width,
		"height", &SpriteComponent::height,
		"start_x", &SpriteComponent::start_x,
		"start_y", &SpriteComponent::start_y,
		"layer", &SpriteComponent::layer,
		"hidden", &SpriteComponent::hidden,
		"uvs", &SpriteComponent::uvs,
		"color", &SpriteComponent::color,
		"generate_uvs", [&](SpriteComponent& sprite)
		{
			auto& assetManager = registry.GetContext<std::shared_ptr<AssetManager>>();
			auto texture = assetManager->GetTexture(sprite.texture_name);

			if (!texture)
			{
				F_ERROR("Failed to generate uvs - texture '{0}' does not exists or invalid!", sprite.texture_name);
				return;
			}

			sprite.generate_uvs(texture->GetWidth(), texture->GetHeight());
		}
	);
}
