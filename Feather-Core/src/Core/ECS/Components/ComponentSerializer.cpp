#include "ComponentSerializer.h"
#include "Core/CoreUtils/CoreUtilities.h"
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
		serializer.StartNewObject("animation")
			.AddKeyValuePair("numFrames", animation.numFrames)
			.AddKeyValuePair("frameRate", animation.frameRate)
			.AddKeyValuePair("frameOffset", animation.frameOffset)
			.AddKeyValuePair("isVertical", animation.isVertical)
			.AddKeyValuePair("isLooped", animation.isLooped)
			.EndObject();
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const BoxColliderComponent& boxCollider)
	{
		serializer.StartNewObject("boxCollider")
			.AddKeyValuePair("width", boxCollider.width)
			.AddKeyValuePair("height", boxCollider.height)
			.StartNewObject("offset")
			.AddKeyValuePair("x", boxCollider.offset.x)
			.AddKeyValuePair("y", boxCollider.offset.y)
			.EndObject()
			.EndObject();
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const CircleColliderComponent& circleCollider)
	{
		serializer.StartNewObject("circleCollider")
			.AddKeyValuePair("radius", circleCollider.radius)
			.StartNewObject("offset")
			.AddKeyValuePair("x", circleCollider.offset.x)
			.AddKeyValuePair("y", circleCollider.offset.y)
			.EndObject()
			.EndObject();
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const TextComponent& text)
	{
		serializer.StartNewObject("text")
			.AddKeyValuePair("text", text.textStr)
			.AddKeyValuePair("fontName", text.fontName)
			.AddKeyValuePair("padding", text.padding)
			.AddKeyValuePair("wrap", text.wrap)
			.AddKeyValuePair("isHidden", text.isHidden)
			.StartNewObject("color")
			.AddKeyValuePair("r", static_cast<int>(text.color.r))
			.AddKeyValuePair("g", static_cast<int>(text.color.g))
			.AddKeyValuePair("b", static_cast<int>(text.color.b))
			.AddKeyValuePair("a", static_cast<int>(text.color.a))
			.EndObject()
			.EndObject();
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const PhysicsComponent& physics)
	{
		const auto& attributes = physics.GetAttributes();
		serializer.StartNewObject("physics")
			.StartNewObject("attributes")
			.AddKeyValuePair("type", GetRigidBodyTypeString(attributes.eType))
			.AddKeyValuePair("density", attributes.density)
			.AddKeyValuePair("friction", attributes.friction)
			.AddKeyValuePair("restitution", attributes.restitution)
			.AddKeyValuePair("restitutionThreshold", attributes.restitutionThreshold)
			.AddKeyValuePair("radius", attributes.radius)
			.AddKeyValuePair("gravityScale", attributes.gravityScale)
			.StartNewObject("position")
			.AddKeyValuePair("x", attributes.position.x)
			.AddKeyValuePair("y", attributes.position.y)
			.EndObject()
			.StartNewObject("scale")
			.AddKeyValuePair("x", attributes.scale.x)
			.AddKeyValuePair("y", attributes.scale.y)
			.EndObject()
			.StartNewObject("boxSize")
			.AddKeyValuePair("x", attributes.boxSize.x)
			.AddKeyValuePair("y", attributes.boxSize.y)
			.EndObject()
			.StartNewObject("offset")
			.AddKeyValuePair("x", attributes.offset.x)
			.AddKeyValuePair("y", attributes.offset.y)
			.EndObject()
			.AddKeyValuePair("isCircle", attributes.isCircle)
			.AddKeyValuePair("isBoxShape", attributes.isBoxShape)
			.AddKeyValuePair("isFixedRotation", attributes.isFixedRotation)
			.AddKeyValuePair("isTrigger", attributes.isTrigger)
			.AddKeyValuePair("filterCategory", static_cast<unsigned>(attributes.filterCategory))
			.AddKeyValuePair("filterMask", static_cast<unsigned>(attributes.filterMask))
			.AddKeyValuePair("groupIndex", static_cast<int>(attributes.groupIndex))
			.StartNewObject("objectData")
			.AddKeyValuePair("tag", attributes.objectData.tag)
			.AddKeyValuePair("group", attributes.objectData.group)
			.AddKeyValuePair("isCollider", attributes.objectData.isCollider)
			.AddKeyValuePair("isTrigger", attributes.objectData.isTrigger)
			.AddKeyValuePair("isFriendly", attributes.objectData.isFriendly)
			.EndObject()
			.EndObject()
			.EndObject();
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const RigidBodyComponent& rigidBody)
	{
		serializer.StartNewObject("rigidBody")
			.StartNewObject("velocity")
			.AddKeyValuePair("x", rigidBody.velocity.x)
			.AddKeyValuePair("y", rigidBody.velocity.y)
			.EndObject()
			.EndObject();
	}

	// ======================= Deserialization =======================

	void ComponentSerializer::DeserializeComponent(const rapidjson::Value& jsonValue, TransformComponent& transform)
	{
		transform.position = glm::vec2{
				jsonValue["position"]["x"].GetFloat(),
				jsonValue["position"]["y"].GetFloat()
		};
		transform.scale = glm::vec2{
				jsonValue["scale"]["x"].GetFloat(),
				jsonValue["scale"]["y"].GetFloat()
		};
		transform.rotation = jsonValue["rotation"].GetFloat();
	}

	void ComponentSerializer::DeserializeComponent(const rapidjson::Value& jsonValue, SpriteComponent& sprite)
	{
		sprite.width = jsonValue["width"].GetFloat();
		sprite.height = jsonValue["height"].GetFloat();
		sprite.uvs = UVs{
			.u = jsonValue["uvs"]["u"].GetFloat(),
			.v = jsonValue["uvs"]["v"].GetFloat(),
			.uv_width = jsonValue["uvs"]["uv_width"].GetFloat(),
			.uv_height = jsonValue["uvs"]["uv_height"].GetFloat()
		};
		sprite.start_x = jsonValue["startX"].GetInt();
		sprite.start_y = jsonValue["startY"].GetInt();
		sprite.layer = jsonValue["layer"].GetInt();
		sprite.isHidden = jsonValue["isHidden"].GetBool();
		sprite.texture_name = jsonValue["texture_name"].GetString();
	}

	void ComponentSerializer::DeserializeComponent(const rapidjson::Value& jsonValue, AnimationComponent& animation)
	{
		animation.numFrames = jsonValue["numFrames"].GetInt();
		animation.frameRate = jsonValue["frameRate"].GetInt();
		animation.frameOffset = jsonValue["frameOffset"].GetInt();
		animation.isVertical = jsonValue["isVertical"].GetBool();
		animation.isLooped = jsonValue["isLooped"].GetBool();
	}

	void ComponentSerializer::DeserializeComponent(const rapidjson::Value& jsonValue, BoxColliderComponent& boxCollider)
	{
		boxCollider.width = jsonValue["width"].GetInt();
		boxCollider.height = jsonValue["height"].GetInt();
		boxCollider.offset = glm::vec2{ jsonValue["offset"]["x"].GetFloat(), jsonValue["offset"]["y"].GetFloat() };
	}

	void ComponentSerializer::DeserializeComponent(const rapidjson::Value& jsonValue, CircleColliderComponent& circleCollider)
	{
		circleCollider.radius = jsonValue["radius"].GetFloat();
		circleCollider.offset = glm::vec2{ jsonValue["offset"]["x"].GetFloat(), jsonValue["offset"]["y"].GetFloat() };
	}

	void ComponentSerializer::DeserializeComponent(const rapidjson::Value& jsonValue, TextComponent& text)
	{
		text.textStr = jsonValue["text"].GetString();
		text.fontName = jsonValue["fontName"].GetString();
		text.padding = jsonValue["padding"].GetInt();
		text.wrap = jsonValue["wrap"].GetInt();
		text.isHidden = jsonValue["isHidden"].GetBool();
		text.color = Color{ .r = static_cast<GLubyte>(jsonValue["color"]["r"].GetInt()),
							.g = static_cast<GLubyte>(jsonValue["color"]["g"].GetInt()),
							.b = static_cast<GLubyte>(jsonValue["color"]["b"].GetInt()),
							.a = static_cast<GLubyte>(jsonValue["color"]["a"].GetInt())
		};
	}

	void ComponentSerializer::DeserializeComponent(const rapidjson::Value& jsonValue, PhysicsComponent& physics)
	{
	}

	void ComponentSerializer::DeserializeComponent(const rapidjson::Value& jsonValue, RigidBodyComponent& rigidBody)
	{
		rigidBody.velocity.x = jsonValue["velocity"]["x"].GetFloat();
		rigidBody.velocity.y = jsonValue["velocity"]["x"].GetFloat();
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
