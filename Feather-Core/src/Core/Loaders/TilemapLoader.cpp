#include "TilemapLoader.h"
#include "Core/ECS/Components/ComponentSerializer.h"
#include "Core/ECS/Registry.h"
#include "Core/ECS/Entity.h"
#include "FileSystem/Serializers/JSONSerializer.h"
#include "Logger/Logger.h"

#include "rapidjson/error/en.h"

#include <filesystem>

namespace Feather {

	bool TilemapLoader::SaveTilemap(Registry& registry, const std::string& tilemapFile, bool useJson)
	{
		if (useJson)
			return SaveTilemapJSON(registry, tilemapFile);

		return false;
	}

	bool TilemapLoader::LoadTilemap(Registry& registry, const std::string& tilemapFile, bool useJson)
	{
		if (useJson)
			return LoadTilemapJSON(registry, tilemapFile);

		return false;
	}

	bool TilemapLoader::SaveTilemapJSON(Registry& registry, const std::string& tilemapFile)
	{
		std::unique_ptr<JSONSerializer> serializer{ nullptr };

		try
		{
			serializer = std::make_unique<JSONSerializer>(tilemapFile);
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to save tilemap '{}': {}", tilemapFile, ex.what());
			return false;
		}

		std::filesystem::path tilemapPath{ tilemapFile };
		if (!std::filesystem::exists(tilemapPath))
		{
			F_ERROR("Failed to save tilemap: Filepath '{}' does not exist", tilemapFile);
			return false;
		}

		serializer->StartDocument();
		serializer->StartNewArray("tilemap");

		auto tiles = registry.GetRegistry().view<TileComponent>();

		for (auto tile : tiles)
		{
			serializer->StartNewObject();
			serializer->StartNewObject("components");
			auto tileEnt{ Entity{registry, tile} };

			const auto& transform = tileEnt.GetComponent<TransformComponent>();
			SERIALIZE_COMPONENT(*serializer, transform);
			const auto& sprite = tileEnt.GetComponent<SpriteComponent>();
			SERIALIZE_COMPONENT(*serializer, sprite);

			if (tileEnt.HasComponent<BoxColliderComponent>())
			{
				const auto& boxCollider = tileEnt.GetComponent<BoxColliderComponent>();
				SERIALIZE_COMPONENT(*serializer, boxCollider);
			}
			if (tileEnt.HasComponent<CircleColliderComponent>())
			{
				const auto& circleCollider = tileEnt.GetComponent<CircleColliderComponent>();
				SERIALIZE_COMPONENT(*serializer, circleCollider);
			}
			if (tileEnt.HasComponent<AnimationComponent>())
			{
				const auto& animation = tileEnt.GetComponent<AnimationComponent>();
				SERIALIZE_COMPONENT(*serializer, animation);
			}
			if (tileEnt.HasComponent<PhysicsComponent>())
			{
				const auto& physics = tileEnt.GetComponent<PhysicsComponent>();
				SERIALIZE_COMPONENT(*serializer, physics);
			}

			serializer->EndObject();
			serializer->EndObject();
		}

		serializer->EndArray();
		return serializer->EndDocument();
	}

	bool TilemapLoader::LoadTilemapJSON(Registry& registry, const std::string& tilemapFile)
	{
		std::ifstream mapFile;
		mapFile.open(tilemapFile);

		if (!mapFile.is_open())
		{
			F_ERROR("Failed to open tilemap file '{}'", tilemapFile);
			return false;
		}

		std::stringstream ss;
		ss << mapFile.rdbuf();
		std::string contents = ss.str();
		rapidjson::StringStream jsonStr{ contents.c_str() };

		rapidjson::Document doc;
		doc.ParseStream(jsonStr);

		if (doc.HasParseError() || !doc.IsObject())
		{
			F_ERROR("Failed to load tilemap file. '{}' is not valid JSON: {} - {}", tilemapFile, rapidjson::GetParseError_En(doc.GetParseError()), doc.GetErrorOffset());
			return false;
		}

		const rapidjson::Value& tilemap = doc["tilemap"];
		if (!tilemap.IsArray() || tilemap.Size() < 1)
		{
			F_ERROR("Failed to load tilemap '{}': there needs to be at least 1 tile", tilemapFile);
			return false;
		}

		for (const auto& tile : tilemap.GetArray())
		{
			Entity newTile{ registry, "", "" };
			const auto& components = tile["components"];

			const auto& jsonTransform = components["transform"];
			auto& transform = newTile.AddComponent<TransformComponent>();
			DESERIALIZE_COMPONENT(jsonTransform, transform);

			const auto& jsonSprite = components["sprite"];
			auto& sprite = newTile.AddComponent<SpriteComponent>();
			DESERIALIZE_COMPONENT(jsonSprite, sprite);

			if (components.HasMember("boxCollider"))
			{
				const auto& jsonBoxCollider = components["boxCollider"];
				auto& boxCollider = newTile.AddComponent<BoxColliderComponent>();
				DESERIALIZE_COMPONENT(jsonBoxCollider, boxCollider);
			}

			if (components.HasMember("circleCollider"))
			{
				const auto& jsonCircleCollider = components["circleCollider"];
				auto& circleCollider = newTile.AddComponent<CircleColliderComponent>();
				DESERIALIZE_COMPONENT(jsonCircleCollider, circleCollider);
			}

			if (components.HasMember("animation"))
			{
				const auto& jsonAnimation = components["animation"];
				auto& animation = newTile.AddComponent<AnimationComponent>();
				DESERIALIZE_COMPONENT(jsonAnimation, animation);
			}

			if (components.HasMember("physics"))
			{
				const auto& jsonPhysics = components["physics"];
				auto& physics = newTile.AddComponent<PhysicsComponent>();
				DESERIALIZE_COMPONENT(jsonPhysics, physics);
			}
		}

		mapFile.close();
		return true;
	}

}
