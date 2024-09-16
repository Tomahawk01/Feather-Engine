#include "ComponentSerializer.h"
#include "FileSystem/Serializers/JSONSerializer.h"

namespace Feather {

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const TransformComponent& transform)
	{
		serializer.StartNewObject("transform")
			.StartNewObject("position")
			.AddKeyValuePair("x", transform.position.x)
			.AddKeyValuePair("y", transform.position.y)
			.EndObject()
			.StartNewObject("scale")
			.AddKeyValuePair("x", transform.scale.x)
			.AddKeyValuePair("y", transform.scale.y)
			.EndObject()
			.AddKeyValuePair("rotation", transform.rotation)
			.EndObject();
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const SpriteComponent& sprite)
	{
		serializer.StartNewObject("sprite")
			.AddKeyValuePair("width", sprite.width)
			.AddKeyValuePair("height", sprite.height)
			.AddKeyValuePair("startX", sprite.start_x)
			.AddKeyValuePair("startY", sprite.start_y)
			.AddKeyValuePair("layer", sprite.layer)
			.AddKeyValuePair("texture_name", sprite.texture_name)
			.StartNewObject("uvs")
			.AddKeyValuePair("u", sprite.uvs.u)
			.AddKeyValuePair("v", sprite.uvs.v)
			.AddKeyValuePair("uv_width", sprite.uvs.uv_width)
			.AddKeyValuePair("uv_height", sprite.uvs.uv_height)
			.EndObject()
			.StartNewObject("color")
			.AddKeyValuePair("r", sprite.color.r)
			.AddKeyValuePair("g", sprite.color.g)
			.AddKeyValuePair("b", sprite.color.b)
			.AddKeyValuePair("a", sprite.color.a)
			.EndObject()
			.AddKeyValuePair("isHidden", sprite.isHidden)
			.EndObject();
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const AnimationComponent& animation)
	{
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const BoxColliderComponent& boxCollider)
	{
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const CircleColliderComponent& circleCollider)
	{
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const TextComponent& text)
	{
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const PhysicsComponent& physics)
	{
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const RigidBodyComponent& rigidBody)
	{
	}

	TransformComponent ComponentSerializer::DeserializeTransform(const rapidjson::Value& jsonValue)
	{
		return TransformComponent{
			.position = glm::vec2{
				jsonValue["position"]["x"].GetFloat(),
				jsonValue["position"]["y"].GetFloat()
			},
			.scale = glm::vec2{
				jsonValue["scale"]["x"].GetFloat(),
				jsonValue["scale"]["y"].GetFloat()
			},
			.rotation = jsonValue["rotation"].GetFloat()
		};
	}

	SpriteComponent ComponentSerializer::DeserializeSprite(const rapidjson::Value& jsonValue)
	{
		return SpriteComponent{
			.width = jsonValue["width"].GetFloat(),
			.height = jsonValue["height"].GetFloat(),
			.uvs = UVs{
				.u = jsonValue["uvs"]["u"].GetFloat(),
				.v = jsonValue["uvs"]["v"].GetFloat(),
				.uv_width = jsonValue["uvs"]["uv_width"].GetFloat(),
				.uv_height = jsonValue["uvs"]["uv_height"].GetFloat()
			},
			.start_x = jsonValue["startX"].GetInt(),
			.start_y = jsonValue["startY"].GetInt(),
			.layer = jsonValue["layer"].GetInt(),
			.isHidden = jsonValue["isHidden"].GetBool(),
			.texture_name = jsonValue["texture_name"].GetString()			
		};
	}

	AnimationComponent ComponentSerializer::DeserializeAnimation(const rapidjson::Value& jsonValue)
	{
		return AnimationComponent();
	}

	BoxColliderComponent ComponentSerializer::DeserializeBoxCollider(const rapidjson::Value& jsonValue)
	{
		return BoxColliderComponent();
	}

	CircleColliderComponent ComponentSerializer::DeserializeCircleCollider(const rapidjson::Value& jsonValue)
	{
		return CircleColliderComponent();
	}

	TextComponent ComponentSerializer::DeserializeText(const rapidjson::Value& jsonValue)
	{
		return TextComponent();
	}

	PhysicsComponent ComponentSerializer::DeserializePhysics(const rapidjson::Value& jsonValue)
	{
		return PhysicsComponent();
	}

	RigidBodyComponent ComponentSerializer::DeserializeRigidBody(const rapidjson::Value& jsonValue)
	{
		return RigidBodyComponent();
	}

}
