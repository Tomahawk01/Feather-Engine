#include "Prefab.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/Entity.h"
#include "Core/ECS/ECSUtils.h"
#include "Core/ECS/Components/ComponentSerializer.h"
#include "Core/CoreUtils/ProjectInfo.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Core/Scene/Scene.h"
#include "Core/Resources/AssetManager.h"
#include "Renderer/Essentials/Texture.h"
#include "FileSystem/Serializers/JSONSerializer.h"
#include "Utils/FeatherUtilities.h"
#include "Utils/HelperUtilities.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

namespace fs = std::filesystem;

namespace Feather {

	Prefab::Prefab()
		: m_Type{ EPrefabType::GameObject }
		, m_Entity{}
		, m_RelatedPrefabs{}
		, m_Name{}
		, m_PrefabPath{}
	{}

	Prefab::Prefab(EPrefabType eType, const PrefabbedEntity& prefabbed)
		: m_Type{ eType }
		, m_Entity{ prefabbed }
		, m_RelatedPrefabs{}
		, m_Name{}
		, m_PrefabPath{}
	{
		F_ASSERT(prefabbed.id && "Must have an ID Component");
		F_ASSERT(!prefabbed.id->name.empty() && "All prefabs must have unique names!");

		m_Name = prefabbed.id->name + "_pfab";
		m_Entity.id->name = m_Name;

		auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
		F_ASSERT(projectInfo && "Project Info must exists!");

		auto optPrefabPath = projectInfo->TryGetFolderPath(EProjectFolderType::Prefabs);
		F_ASSERT(optPrefabPath && "Prefab folder path not set correctly");

		fs::path prefabPath = *optPrefabPath / fs::path{ m_Name + ".json" };

		if (fs::exists(prefabPath))
		{
			F_ERROR("Failed to create prefab. '{}' Already exists!", m_Name);
			throw std::runtime_error(std::format("Failed to create prefab. '{}' Already exists!", m_Name).c_str());
		}

		m_PrefabPath = prefabPath.string();

		Save();
	}

	Prefab::Prefab(const std::string& prefabPath)
		: m_Type{ EPrefabType::GameObject }
		, m_Entity{}
		, m_RelatedPrefabs{}
		, m_Name{}
		, m_PrefabPath{ prefabPath }
	{
		if (!Load(prefabPath))
		{
			F_ERROR("Failed to load prefab from path '{}'", prefabPath);
			throw std::runtime_error(std::format("Failed to load prefab from path '{}'", prefabPath).c_str());
		}
	}

	Prefab::~Prefab()
	{}

	void Prefab::AddChild(const PrefabbedEntity& child)
	{
		auto& prefabbedChild = m_RelatedPrefabs.emplace_back(std::make_shared<PrefabbedEntity>(child));
		if (!m_Entity.relationships)
		{
			m_Entity.relationships = PrefabbedRelationships{};
			m_Entity.relationships->firstChild = prefabbedChild.get();
			return;
		}

		auto& relations = m_Entity.relationships.value();
		if (!prefabbedChild->relationships)
		{
			prefabbedChild->relationships = PrefabbedRelationships{};
		}
		auto& childRelations = prefabbedChild->relationships.value();
		if (auto* prevSibling = childRelations.prevSibling)
		{
			if (prevSibling->relationships)
			{
				prevSibling->relationships->nextSibling = childRelations.nextSibling;
			}
		}

		if (auto* nextSibling = childRelations.nextSibling)
		{
			if (nextSibling->relationships)
			{
				nextSibling->relationships->prevSibling = childRelations.prevSibling;
			}
		}

		if (auto* parent = childRelations.parent)
		{
			if (parent->relationships)
			{
				if (parent->relationships->firstChild == prefabbedChild.get())
				{
					parent->relationships->firstChild = nullptr;
					if (auto* nextSibling = childRelations.nextSibling)
					{
						if (nextSibling->relationships)
						{
							parent->relationships->firstChild = nextSibling;
							nextSibling->relationships->prevSibling = nullptr;
						}
					}
				}
			}
		}

		// Reset the child's siblings
		childRelations.nextSibling = nullptr;
		childRelations.prevSibling = nullptr;

		childRelations.parent = &m_Entity;

		if (!relations.firstChild)
		{
			relations.firstChild = prefabbedChild.get();
		}
		else
		{
			// TODO: Set sibling links
		}
	}

	bool Prefab::Load(const std::string& prefabPath)
	{
		std::ifstream prefabFile;
		prefabFile.open(prefabPath);

		if (!prefabFile.is_open())
		{
			F_ERROR("Failed to open tilemap file '{}'", prefabPath);
			return false;
		}

		// If the prefab file is empty, it is not valid
		if (prefabFile.peek() == std::ifstream::traits_type::eof())
		{
			F_ERROR("Failed to load prefab file '{}' - Empty prefab files are invalid", prefabPath);
			return false;
		}

		std::stringstream ss;
		ss << prefabFile.rdbuf();
		std::string contents = ss.str();
		rapidjson::StringStream jsonStr{ contents.c_str() };

		rapidjson::Document doc;
		doc.ParseStream(jsonStr);

		if (doc.HasParseError() || !doc.IsObject())
		{
			F_ERROR("Failed to load prefab file: '{}' is not valid JSON, Error: {}, Offset: {}", prefabPath, rapidjson::GetParseError_En(doc.GetParseError()), doc.GetErrorOffset());
			return false;
		}

		const rapidjson::Value& prefab = doc["prefab"];
		if (!prefab.HasMember("components"))
		{
			F_ERROR("Failed to load prefab file: '{}' - Prefabs must have components", prefabPath);
			return false;
		}

		const rapidjson::Value& components = prefab["components"];

		if (components.HasMember("id"))
		{
			const rapidjson::Value& id = components["id"];
			Identification idComp{};
			DESERIALIZE_COMPONENT(id, idComp);
			m_Entity.id = idComp;

			m_Name = m_Entity.id->name;
		}

		if (components.HasMember("transform"))
		{
			// The transform position does not matter
			const rapidjson::Value& transform = components["transform"];
			m_Entity.transform.scale = glm::vec2{ transform["scale"]["x"].GetFloat(), transform["scale"]["y"].GetFloat() };
			m_Entity.transform.rotation = transform["rotation"].GetFloat();
		}

		if (components.HasMember("sprite"))
		{
			const rapidjson::Value& sprite = components["sprite"];
			SpriteComponent spriteComp{};
			DESERIALIZE_COMPONENT(sprite, spriteComp);
			m_Entity.sprite = spriteComp;
		}

		if (components.HasMember("animation"))
		{
			const rapidjson::Value& animation = components["animation"];
			AnimationComponent animationComp{};
			DESERIALIZE_COMPONENT(animation, animationComp);
			m_Entity.animation = animationComp;
		}

		if (components.HasMember("boxCollider"))
		{
			const rapidjson::Value& boxCollider = components["boxCollider"];
			BoxColliderComponent boxColliderComp{};
			DESERIALIZE_COMPONENT(boxCollider, boxColliderComp);
			m_Entity.boxCollider = boxColliderComp;
		}

		if (components.HasMember("circleCollider"))
		{
			const rapidjson::Value& circleCollider = components["circleCollider"];
			CircleColliderComponent circleColliderComp{};
			DESERIALIZE_COMPONENT(circleCollider, circleColliderComp);
			m_Entity.circleCollider = circleColliderComp;
		}

		if (components.HasMember("physics"))
		{
			const rapidjson::Value& physics = components["physics"];
			PhysicsComponent physicsComp{};
			DESERIALIZE_COMPONENT(physics, physicsComp);
			m_Entity.physics = physicsComp;
		}

		if (components.HasMember("text"))
		{
			const rapidjson::Value& text = components["text"];
			TextComponent textComp{};
			DESERIALIZE_COMPONENT(text, textComp);
			m_Entity.textComp = textComp;
		}

		// TODO: All other components
		prefabFile.close();

		return true;
	}

	bool Prefab::Save()
	{
		std::unique_ptr<JSONSerializer> pSerializer{ nullptr };

		try
		{
			pSerializer = std::make_unique<JSONSerializer>(m_PrefabPath);
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to save game objects '{}' - {}", m_PrefabPath, ex.what());
			return false;
		}

		fs::path prefabPath{ m_PrefabPath };
		if (!fs::exists(prefabPath))
		{
			F_ERROR("Failed to save game objects - Filepath '{}' does not exist", m_PrefabPath);
			return false;
		}

		pSerializer->StartDocument();
		pSerializer->StartNewObject("prefab");

		pSerializer->StartNewObject("components");

		SERIALIZE_COMPONENT(*pSerializer, m_Entity.transform);
		const auto& id = m_Entity.id.value();
		SERIALIZE_COMPONENT(*pSerializer, id);

		if (m_Entity.sprite)
		{
			const auto& sprite = m_Entity.sprite.value();
			SERIALIZE_COMPONENT(*pSerializer, sprite);
		}

		if (m_Entity.animation)
		{
			const auto& animation = m_Entity.animation.value();
			SERIALIZE_COMPONENT(*pSerializer, animation);
		}

		if (m_Entity.boxCollider)
		{
			const auto& boxCollider = m_Entity.boxCollider.value();
			SERIALIZE_COMPONENT(*pSerializer, boxCollider);
		}

		if (m_Entity.circleCollider)
		{
			const auto& circleCollider = m_Entity.circleCollider.value();
			SERIALIZE_COMPONENT(*pSerializer, circleCollider);
		}

		if (m_Entity.physics)
		{
			const auto& physics = m_Entity.physics.value();
			SERIALIZE_COMPONENT(*pSerializer, physics);
		}

		if (m_Entity.rigidBody)
		{
			const auto& rigidBody = m_Entity.rigidBody.value();
			SERIALIZE_COMPONENT(*pSerializer, rigidBody);
		}

		if (m_Entity.textComp)
		{
			const auto& textComp = m_Entity.textComp.value();
			SERIALIZE_COMPONENT(*pSerializer, textComp);
		}

		if (m_Entity.uiComp)
		{
			const auto& ui = m_Entity.uiComp.value();
			SERIALIZE_COMPONENT( *pSerializer, ui );
		}

		pSerializer->EndObject(); // End Components
		pSerializer->EndObject(); // End Prefab
		pSerializer->EndDocument();
		return true;
	}	

	std::shared_ptr<Prefab> PrefabCreator::CreatePrefab(EPrefabType type, Entity& entityToPrefab)
	{
		PrefabbedEntity prefabbed{};

		if (auto* transform = entityToPrefab.TryGetComponent<TransformComponent>())
		{
			prefabbed.transform = *transform;
			prefabbed.transform.position = glm::vec2{ 0.0f };
		}

		if (auto* sprite = entityToPrefab.TryGetComponent<SpriteComponent>())
		{
			prefabbed.sprite = *sprite;
			auto texture = ASSET_MANAGER().GetTexture(sprite->textureName);
			F_ASSERT(texture && "Sprite texture must exist in the asset manager");
			GenerateUVs(*prefabbed.sprite, texture->GetWidth(), texture->GetHeight());
		}

		if (auto* boxCollider = entityToPrefab.TryGetComponent<BoxColliderComponent>())
		{
			prefabbed.boxCollider = *boxCollider;
		}

		if (auto* circleCollider = entityToPrefab.TryGetComponent<CircleColliderComponent>())
		{
			prefabbed.circleCollider = *circleCollider;
		}

		if (auto* physics = entityToPrefab.TryGetComponent<PhysicsComponent>())
		{
			prefabbed.physics = *physics;
		}

		if (auto* animation = entityToPrefab.TryGetComponent<AnimationComponent>())
		{
			prefabbed.animation = *animation;
		}

		if (auto* rigidBody = entityToPrefab.TryGetComponent<RigidBodyComponent>())
		{
			prefabbed.rigidBody = *rigidBody;
		}

		if (auto* id = entityToPrefab.TryGetComponent<Identification>())
		{
			prefabbed.id = *id;
		}

		if (auto* ui = entityToPrefab.TryGetComponent<UIComponent>())
		{
			prefabbed.uiComp = *ui;
		}

		if (auto* text = entityToPrefab.TryGetComponent<TextComponent>())
		{
			prefabbed.textComp = *text;
		}

		if (auto* relations = entityToPrefab.TryGetComponent<Relationship>())
		{
			// TODO: handle relationships
		}

		try
		{
			return std::make_shared<Prefab>(type, prefabbed);
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to create prefab. {}", ex.what());
		}

		return nullptr;
	}

	std::shared_ptr<Prefab> PrefabCreator::CreatePrefab(const std::string& prefabPath)
	{
		try
		{
			return std::make_shared<Prefab>(prefabPath);
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to create prefab. {}", ex.what());
		}

		return nullptr;
	}

	std::shared_ptr<Entity> PrefabCreator::AddPrefabToScene(const Prefab& prefab, Registry& registry)
	{
		const auto& prefabbed = prefab.GetPrefabbedEntity();
		
		// Remove the _pfab from the prefabbed id
		std::string tag{ RemoveSuffixCopy(prefabbed.id->name, "_pfab") };
		std::string checkTag{ tag };
		int current{ 0 };

		entt::entity hasEntity = FindEntityByTag(registry, tag);
		while (hasEntity != entt::null)
		{
			checkTag = tag + std::to_string(current);
			hasEntity = FindEntityByTag(registry, tag);
			++current;
		}

		tag = checkTag;

		auto newEnt = std::make_shared<Entity>(registry, tag, prefabbed.id->group);

		newEnt->AddComponent<TransformComponent>(prefabbed.transform);
		if (prefabbed.sprite)
		{
			newEnt->AddComponent<SpriteComponent>(prefabbed.sprite.value());
		}

		if (prefabbed.animation)
		{
			newEnt->AddComponent<AnimationComponent>(prefabbed.animation.value());
		}

		if (prefabbed.boxCollider)
		{
			newEnt->AddComponent<BoxColliderComponent>(prefabbed.boxCollider.value());
		}

		if (prefabbed.circleCollider)
		{
			newEnt->AddComponent<CircleColliderComponent>(prefabbed.circleCollider.value());
		}

		if (prefabbed.textComp)
		{
			newEnt->AddComponent<TextComponent>(prefabbed.textComp.value());
		}

		if (prefabbed.physics)
		{
			newEnt->AddComponent<PhysicsComponent>(prefabbed.physics.value());
		}

		return newEnt;
	}

	bool PrefabCreator::DeletePrefab(Prefab& prefabToDelete)
	{
		fs::path prefabPath{ prefabToDelete.GetFilepath() };

		std::error_code ec;
		if (!fs::exists(prefabPath, ec))
		{
			F_ERROR("Failed to delete prefab. {}", ec.message());
			return false;
		}

		if (fs::is_directory(prefabPath) || !prefabPath.has_extension())
		{
			F_ERROR("Failed to delete prefab. The path is a directory. To delete, we must pass in the file");
			return false;
		}

		if (!fs::remove(prefabPath, ec))
		{
			F_ERROR("Failed to delete prefab. {}", ec.message());
			return false;
		}

		// Recheck to ensure file was deleted
		if (fs::exists(prefabPath))
		{
			F_ERROR("Failed to delete prefab. File not deleted correctly");
			return false;
		}

		return true;
	}

}
