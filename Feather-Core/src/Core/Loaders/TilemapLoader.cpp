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

	bool TilemapLoader::LoadGameObjects(Registry& registry, const std::string& objectMapFile, bool useJSON)
	{
		if (useJSON)
			return LoadObjectMapJSON(registry, objectMapFile);

		return false;
	}

	bool TilemapLoader::SaveGameObjects(Registry& registry, const std::string& objectMapFile, bool useJSON)
	{
		if (useJSON)
			return SaveObjectMapJSON(registry, objectMapFile);

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

		// The tilemap file could be empty if just created
		if (mapFile.peek() == std::ifstream::traits_type::eof())
			return true;

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
			F_WARN("Failed to load tilemap '{}': there needs to be at least 1 tile", tilemapFile);
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

			newTile.AddComponent<TileComponent>(TileComponent{ .id = static_cast<uint32_t>(newTile.GetEntity()) });
		}

		mapFile.close();
		return true;
	}

	bool TilemapLoader::SaveObjectMapJSON(Registry& registry, const std::string& objectMapFile)
	{
		std::unique_ptr<JSONSerializer> serializer{ nullptr };

		try
		{
			serializer = std::make_unique<JSONSerializer>(objectMapFile);
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to save game objects '{}': {}", objectMapFile, ex.what());
			return false;
		}

		std::filesystem::path objectPath{ objectMapFile };
		if (!std::filesystem::exists(objectPath))
		{
			F_ERROR("Failed to save game objects: Filepath does not exist '{}'", objectMapFile);
			return false;
		}

		serializer->StartDocument();
		serializer->StartNewArray("game_objects");

		auto gameObjects = registry.GetRegistry().view<entt::entity>(entt::exclude<TileComponent, UneditableComponent>);

		for (auto object : gameObjects)
		{
			serializer->StartNewObject();
			serializer->StartNewObject("components");

			auto objectEnt{ Entity{ registry, object } };

			if (const auto* id = objectEnt.TryGetComponent<Identification>())
			{
				SERIALIZE_COMPONENT(*serializer, *id);
			}
			if (const auto* transform = objectEnt.TryGetComponent<TransformComponent>())
			{
				SERIALIZE_COMPONENT(*serializer, *transform);
			}
			if (const auto* sprite = objectEnt.TryGetComponent<SpriteComponent>())
			{
				SERIALIZE_COMPONENT(*serializer, *sprite);
			}

			if (objectEnt.HasComponent<BoxColliderComponent>())
			{
				const auto& boxCollider = objectEnt.GetComponent<BoxColliderComponent>();
				SERIALIZE_COMPONENT(*serializer, boxCollider);
			}
			if (objectEnt.HasComponent<CircleColliderComponent>())
			{
				const auto& circleCollider = objectEnt.GetComponent<CircleColliderComponent>();
				SERIALIZE_COMPONENT(*serializer, circleCollider);
			}
			if (objectEnt.HasComponent<AnimationComponent>())
			{
				const auto& animation = objectEnt.GetComponent<AnimationComponent>();
				SERIALIZE_COMPONENT(*serializer, animation);
			}
			if (objectEnt.HasComponent<PhysicsComponent>())
			{
				const auto& physics = objectEnt.GetComponent<PhysicsComponent>();
				SERIALIZE_COMPONENT(*serializer, physics);
			}
			if (objectEnt.HasComponent<TextComponent>())
			{
				const auto& text = objectEnt.GetComponent<TextComponent>();
				SERIALIZE_COMPONENT(*serializer, text);
			}

			if (auto* relations = objectEnt.TryGetComponent<Relationship>())
			{
				serializer->StartNewObject("relationship");
				if (relations->parent != entt::null)
				{
					Entity parent{ registry, relations->parent };
					serializer->AddKeyValuePair("parent", parent.GetName());
				}
				else
				{
					serializer->AddKeyValuePair("parent", std::string{ "" });
				}

				if (relations->nextSibling != entt::null)
				{
					Entity nextSibling{ registry, relations->nextSibling };
					serializer->AddKeyValuePair("nextSibling", nextSibling.GetName());
				}
				else
				{
					serializer->AddKeyValuePair("nextSibling", std::string{ "" });
				}

				if (relations->prevSibling != entt::null)
				{
					Entity prevSibling{ registry, relations->prevSibling };
					serializer->AddKeyValuePair("prevSibling", prevSibling.GetName());
				}
				else
				{
					serializer->AddKeyValuePair("prevSibling", std::string{ "" });
				}

				if (relations->firstChild != entt::null)
				{
					Entity firstChild{ registry, relations->firstChild };
					serializer->AddKeyValuePair("firstChild", firstChild.GetName());
				}
				else
				{
					serializer->AddKeyValuePair("firstChild", std::string{ "" });
				}
				serializer->EndObject(); // Relationship Object
			}

			serializer->EndObject(); // Components object
			serializer->EndObject(); // Ent GameObject object
		}

		serializer->EndArray(); // GameObjects array
		return serializer->EndDocument();
	}

	bool TilemapLoader::LoadObjectMapJSON(Registry& registry, const std::string& objectMapFile)
	{
		std::ifstream mapFile;
		mapFile.open(objectMapFile);

		if (!mapFile.is_open())
		{
			F_ERROR("Failed to open tilemap file '{}'", objectMapFile);
			return false;
		}

		// The object file could be empty if just created
		if (mapFile.peek() == std::ifstream::traits_type::eof())
			return true;

		std::stringstream ss;
		ss << mapFile.rdbuf();
		std::string contents = ss.str();
		rapidjson::StringStream jsonStr{ contents.c_str() };

		rapidjson::Document doc;
		doc.ParseStream(jsonStr);

		if (doc.HasParseError() || !doc.IsObject())
		{
			F_ERROR("Failed to load tilemap: File '{}' is not valid JSON - {} - {}",
				objectMapFile,
				rapidjson::GetParseError_En(doc.GetParseError()),
				doc.GetErrorOffset());
			return false;
		}

		const rapidjson::Value& gameObjects = doc["game_objects"];
		if (!gameObjects.IsArray() || gameObjects.Size() < 1)
		{
			F_WARN("Failed to load Game Objects: File '{}' - There needs to be at least 1 tile", objectMapFile);
			return false;
		}

		// Map of entity to relationships
		std::map<entt::entity, SaveRelationship> mapEntityToRelationship;

		for (const auto& object : gameObjects.GetArray())
		{
			Entity gameObject{ registry, "", "" };
			const auto& components = object["components"];

			// Transform
			const auto& jsonTransform = components["transform"];
			auto& transform = gameObject.AddComponent<TransformComponent>();
			DESERIALIZE_COMPONENT(jsonTransform, transform);

			// Sprite. We CAN'T assume that all game objects have a sprite
			if (components.HasMember("sprite"))
			{
				auto& jsonSprite = components["sprite"];
				auto& sprite = gameObject.AddComponent<SpriteComponent>();
				DESERIALIZE_COMPONENT(jsonSprite, sprite);
			}
			if (components.HasMember("boxCollider"))
			{
				const auto& jsonBoxCollider = components["boxCollider"];
				auto& boxCollider = gameObject.AddComponent<BoxColliderComponent>();
				DESERIALIZE_COMPONENT(jsonBoxCollider, boxCollider);
			}
			if (components.HasMember("circleCollider"))
			{
				const auto& jsonCircleCollider = components["circleCollider"];
				auto& circleCollider = gameObject.AddComponent<CircleColliderComponent>();
				DESERIALIZE_COMPONENT(jsonCircleCollider, circleCollider);
			}
			if (components.HasMember("animation"))
			{
				const auto& jsonAnimation = components["animation"];
				auto& animation = gameObject.AddComponent<AnimationComponent>();
				DESERIALIZE_COMPONENT(jsonAnimation, animation);
			}
			if (components.HasMember("physics"))
			{
				const auto& jsonPhysics = components["physics"];
				auto& physics = gameObject.AddComponent<PhysicsComponent>();
				DESERIALIZE_COMPONENT(jsonPhysics, physics);
			}
			if (components.HasMember("id"))
			{
				const auto& jsonID = components["id"];
				auto& id = gameObject.GetComponent<Identification>();
				DESERIALIZE_COMPONENT(jsonID, id);
			}
			if (components.HasMember("text"))
			{
				const auto& jsonText = components["text"];
				auto& text = gameObject.AddComponent<TextComponent>();
				DESERIALIZE_COMPONENT(jsonText, text);
			}

			if (components.HasMember("relationship"))
			{
				const rapidjson::Value& relations = components["relationship"];
				SaveRelationship saveRelations{};
				saveRelations.Parent = relations["parent"].GetString();
				saveRelations.NextSibling = relations["nextSibling"].GetString();
				saveRelations.PrevSibling = relations["prevSibling"].GetString();
				saveRelations.FirstChild = relations["firstChild"].GetString();

				mapEntityToRelationship.emplace(gameObject.GetEntity(), saveRelations);
			}

			auto ids = registry.GetRegistry().view<Identification>(entt::exclude<TileComponent>);

			auto findTag = [&](const std::string& sTag) {
				auto parItr = std::ranges::find_if(ids, [&](const auto& e) {
					Entity en{ registry, e };
					return en.GetName() == sTag;
				});

				if (parItr != ids.end())
					return *parItr;

				return entt::entity{ entt::null };
			};

			for (auto& [entity, saveRelations] : mapEntityToRelationship)
			{
				Entity ent{ registry, entity };
				auto& relations = ent.GetComponent<Relationship>();

				if (!saveRelations.Parent.empty())
				{
					relations.parent = findTag(saveRelations.Parent);
				}

				if (!saveRelations.NextSibling.empty())
				{
					relations.nextSibling = findTag(saveRelations.NextSibling);
				}

				if (!saveRelations.PrevSibling.empty())
				{
					relations.prevSibling = findTag(saveRelations.PrevSibling);
				}

				if (!saveRelations.FirstChild.empty())
				{
					relations.firstChild = findTag(saveRelations.FirstChild);
				}
			}
		}

		mapFile.close();
		return true;
	}

}
