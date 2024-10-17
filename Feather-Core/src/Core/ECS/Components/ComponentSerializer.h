#pragma once
#include "AllComponents.h"

#include "rapidjson/document.h"

#define SERIALIZE_COMPONENT(serializer, component) Feather::ComponentSerializer::Serialize(serializer, component)
#define DESERIALIZE_COMPONENT(table, compref) Feather::ComponentSerializer::Deserialize(table, compref)

namespace Feather {

	class JSONSerializer;

	class ComponentSerializer
	{
	public:
		ComponentSerializer() = delete;

		template <typename TComponent, typename TSerializer>
		static void Serialize(TSerializer& serializer, const TComponent& component);

		template <typename TComponent, typename TTable>
		static void Deserialize(const TTable& table, TComponent& component);

	private:
		static void SerializeComponent(JSONSerializer& serializer, const TransformComponent& transform);
		static void SerializeComponent(JSONSerializer& serializer, const SpriteComponent& sprite);
		static void SerializeComponent(JSONSerializer& serializer, const AnimationComponent& animation);
		static void SerializeComponent(JSONSerializer& serializer, const BoxColliderComponent& boxCollider);
		static void SerializeComponent(JSONSerializer& serializer, const CircleColliderComponent& circleCollider);
		static void SerializeComponent(JSONSerializer& serializer, const TextComponent& text);
		static void SerializeComponent(JSONSerializer& serializer, const PhysicsComponent& physics);
		static void SerializeComponent(JSONSerializer& serializer, const RigidBodyComponent& rigidBody);

		static void DeserializeComponent(const rapidjson::Value& jsonValue, TransformComponent& transform);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, SpriteComponent& sprite);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, AnimationComponent& animation);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, BoxColliderComponent& boxCollider);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, CircleColliderComponent& circleCollider);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, TextComponent& text);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, PhysicsComponent& physics);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, RigidBodyComponent& rigidBody);

		[[deprecated]]
		static TransformComponent DeserializeTransform(const rapidjson::Value& jsonValue);
		[[deprecated]]
		static SpriteComponent DeserializeSprite(const rapidjson::Value& jsonValue);
		[[deprecated]]
		static AnimationComponent DeserializeAnimation(const rapidjson::Value& jsonValue);
		[[deprecated]]
		static BoxColliderComponent DeserializeBoxCollider(const rapidjson::Value& jsonValue);
		[[deprecated]]
		static CircleColliderComponent DeserializeCircleCollider(const rapidjson::Value& jsonValue);
		[[deprecated]]
		static TextComponent DeserializeText(const rapidjson::Value& jsonValue);
		[[deprecated]]
		static PhysicsComponent DeserializePhysics(const rapidjson::Value& jsonValue);
		[[deprecated]]
		static RigidBodyComponent DeserializeRigidBody(const rapidjson::Value& jsonValue);
	};

}

#include "ComponentSerializer.inl"
