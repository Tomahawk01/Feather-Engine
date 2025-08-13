#pragma once

#include "AllComponents.h"

#include <rapidjson/document.h>
#include <sol/sol.hpp>

#define SERIALIZE_COMPONENT(serializer, component) Feather::ComponentSerializer::Serialize(serializer, component)
#define DESERIALIZE_COMPONENT(table, compref) Feather::ComponentSerializer::Deserialize(table, compref)

namespace Feather {

	class JSONSerializer;
	class LuaSerializer;

	class ComponentSerializer
	{
	public:
		ComponentSerializer() = delete;

		template <typename TComponent, typename TSerializer>
		static void Serialize(TSerializer& serializer, const TComponent& component);

		template <typename TComponent, typename TTable>
		static void Deserialize(const TTable& table, TComponent& component);

	private:
		// NOTE: JSON serializer
		static void SerializeComponent(JSONSerializer& serializer, const TransformComponent& transform);
		static void SerializeComponent(JSONSerializer& serializer, const SpriteComponent& sprite);
		static void SerializeComponent(JSONSerializer& serializer, const AnimationComponent& animation);
		static void SerializeComponent(JSONSerializer& serializer, const BoxColliderComponent& boxCollider);
		static void SerializeComponent(JSONSerializer& serializer, const CircleColliderComponent& circleCollider);
		static void SerializeComponent(JSONSerializer& serializer, const TextComponent& text);
		static void SerializeComponent(JSONSerializer& serializer, const PhysicsComponent& physics);
		static void SerializeComponent(JSONSerializer& serializer, const RigidBodyComponent& rigidBody);
		static void SerializeComponent(JSONSerializer& serializer, const Identification& id);
		static void SerializeComponent(JSONSerializer& serializer, const UIComponent& ui);

		static void DeserializeComponent(const rapidjson::Value& jsonValue, TransformComponent& transform);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, SpriteComponent& sprite);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, AnimationComponent& animation);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, BoxColliderComponent& boxCollider);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, CircleColliderComponent& circleCollider);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, TextComponent& text);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, PhysicsComponent& physics);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, RigidBodyComponent& rigidBody);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, Identification& id);
		static void DeserializeComponent(const rapidjson::Value& jsonValue, UIComponent& ui);

		// NOTE: LUA serializer
		static void SerializeComponent(LuaSerializer& serializer, const TransformComponent& transform);
		static void SerializeComponent(LuaSerializer& serializer, const SpriteComponent& sprite);
		static void SerializeComponent(LuaSerializer& serializer, const AnimationComponent& animation);
		static void SerializeComponent(LuaSerializer& serializer, const BoxColliderComponent& boxCollider);
		static void SerializeComponent(LuaSerializer& serializer, const CircleColliderComponent& circleCollider);
		static void SerializeComponent(LuaSerializer& serializer, const TextComponent& text);
		static void SerializeComponent(LuaSerializer& serializer, const PhysicsComponent& physics);
		static void SerializeComponent(LuaSerializer& serializer, const RigidBodyComponent& rigidBody);
		static void SerializeComponent(LuaSerializer& serializer, const Identification& id);
		static void SerializeComponent(LuaSerializer& serializer, const UIComponent& ui);

		static void DeserializeComponent(const sol::table& table, TransformComponent& transform);
		static void DeserializeComponent(const sol::table& table, SpriteComponent& sprite);
		static void DeserializeComponent(const sol::table& table, AnimationComponent& animation);
		static void DeserializeComponent(const sol::table& table, BoxColliderComponent& boxCollider);
		static void DeserializeComponent(const sol::table& table, CircleColliderComponent& circleCollider);
		static void DeserializeComponent(const sol::table& table, TextComponent& text);
		static void DeserializeComponent(const sol::table& table, PhysicsComponent& physics);
		static void DeserializeComponent(const sol::table& table, RigidBodyComponent& rigidBody);
		static void DeserializeComponent(const sol::table& table, Identification& id);
		static void DeserializeComponent(const sol::table& table, UIComponent& ui);
	};

}

#include "ComponentSerializer.inl"
