#pragma once
#include "AllComponents.h"

#include "rapidjson/document.h"

#define SERIALIZE_COMPONENT(serializer, component) Feather::ComponentSerializer::Serialize(serializer, component)
#define DESERIALIZE_COMPONENT(COMP, serializer) Feather::ComponentSerializer::Deserialize<COMP>(serializer)

namespace Feather {

	class JSONSerializer;

	class ComponentSerializer
	{
	public:
		ComponentSerializer() = delete;

		template <typename TComponent, typename TSerializer>
		static void Serialize(TSerializer& serializer, const TComponent& component);

		template <typename TComponent, typename TTable>
		static auto Deserialize(const TTable& table);

	private:
		static void SerializeComponent(JSONSerializer& serializer, const TransformComponent& transform);
		static void SerializeComponent(JSONSerializer& serializer, const SpriteComponent& sprite);
		static void SerializeComponent(JSONSerializer& serializer, const AnimationComponent& animation);
		static void SerializeComponent(JSONSerializer& serializer, const BoxColliderComponent& boxCollider);
		static void SerializeComponent(JSONSerializer& serializer, const CircleColliderComponent& circleCollider);
		static void SerializeComponent(JSONSerializer& serializer, const TextComponent& text);
		static void SerializeComponent(JSONSerializer& serializer, const PhysicsComponent& physics);
		static void SerializeComponent(JSONSerializer& serializer, const RigidBodyComponent& rigidBody);

		static TransformComponent DeserializeTransform(const rapidjson::Value& jsonValue);
		static SpriteComponent DeserializeSprite(const rapidjson::Value& jsonValue);
		static AnimationComponent DeserializeAnimation(const rapidjson::Value& jsonValue);
		static BoxColliderComponent DeserializeBoxCollider(const rapidjson::Value& jsonValue);
		static CircleColliderComponent DeserializeCircleCollider(const rapidjson::Value& jsonValue);
		static TextComponent DeserializeText(const rapidjson::Value& jsonValue);
		static PhysicsComponent DeserializePhysics(const rapidjson::Value& jsonValue);
		static RigidBodyComponent DeserializeRigidBody(const rapidjson::Value& jsonValue);
	};

}

#include "ComponentSerializer.inl"
