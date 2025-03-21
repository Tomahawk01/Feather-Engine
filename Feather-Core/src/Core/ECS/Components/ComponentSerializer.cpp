#include "ComponentSerializer.h"

#include "Core/CoreUtils/CoreUtilities.h"
#include "FileSystem/Serializers/JSONSerializer.h"
#include "Physics/PhysicsUtilities.h"

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
			.StartNewObject("maxVelocity")
			.AddKeyValuePair("x", rigidBody.maxVelocity.x)
			.AddKeyValuePair("y", rigidBody.maxVelocity.y)
			.EndObject()
			.EndObject();
	}

	void ComponentSerializer::SerializeComponent(JSONSerializer& serializer, const Identification& id)
	{
		serializer.StartNewObject("id")
			.AddKeyValuePair("name", id.name)
			.AddKeyValuePair("group", id.group)
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
		if (jsonValue.HasMember("attributes"))
		{
			const auto& attr = jsonValue["attributes"];
			PhysicsAttributes attributes{
				.eType = GetRigidBodyTypeByString(attr["type"].GetString()),
				.density = attr["density"].GetFloat(),
				.friction = attr["friction"].GetFloat(),
				.restitution = attr["restitution"].GetFloat(),
				.restitutionThreshold = attr["restitutionThreshold"].GetFloat(),
				.radius = attr["radius"].GetFloat(),
				.gravityScale = attr["gravityScale"].GetFloat(),
				.position = glm::vec2{ attr["position"]["x"].GetFloat(), attr["position"]["y"].GetFloat() },
				.scale = glm::vec2{ attr["scale"]["x"].GetFloat(), attr["scale"]["y"].GetFloat() },
				.boxSize = glm::vec2{ attr["boxSize"]["x"].GetFloat(), attr["boxSize"]["y"].GetFloat() },
				.offset = glm::vec2{ attr["offset"]["x"].GetFloat(), attr["offset"]["y"].GetFloat() },
				.isCircle = attr["isCircle"].GetBool(),
				.isBoxShape = attr["isBoxShape"].GetBool(),
				.isFixedRotation = attr["isFixedRotation"].GetBool(),
				.isTrigger = attr["isTrigger"].GetBool(),
				.filterCategory = static_cast<uint16_t>(attr["filterCategory"].GetUint()),
				.filterMask = static_cast<uint16_t>(attr["filterMask"].GetUint()),
				.groupIndex = static_cast<int16_t>(attr["groupIndex"].GetInt()),
				.objectData = ObjectData{.tag = attr["objectData"]["tag"].GetString(),
										  .group = attr["objectData"]["group"].GetString(),
										  .isCollider = attr["objectData"]["isCollider"].GetBool(),
										  .isTrigger = attr["objectData"]["isTrigger"].GetBool(),
										  .isFriendly = attr["objectData"]["isFriendly"].GetBool() } };

			physics.GetChangableAttributes() = attributes;
		}
	}

	void ComponentSerializer::DeserializeComponent(const rapidjson::Value& jsonValue, RigidBodyComponent& rigidBody)
	{
		rigidBody.maxVelocity.x = jsonValue["maxVelocity"]["x"].GetFloat();
		rigidBody.maxVelocity.y = jsonValue["maxVelocity"]["y"].GetFloat();
	}

	void ComponentSerializer::DeserializeComponent(const rapidjson::Value& jsonValue, Identification& id)
	{
		id.name = jsonValue["name"].GetString();
		id.group = jsonValue["group"].GetString();
	}

}
