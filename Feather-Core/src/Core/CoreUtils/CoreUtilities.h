#pragma once

#include "Core/ECS/Components/AllComponents.h"
#include "Renderer/Core/Camera2D.h"

namespace Feather {

	bool EntityInView(const TransformComponent& transform, float width, float height, const Camera2D& camera);
	// Transform, Rotation, Scale
	glm::mat4 TRSModel(const TransformComponent& transform, float width, float height);

	std::string GetRigidBodyTypeString(RigidBodyType rigidType);
	RigidBodyType GetRigidBodyTypeByString(const std::string rigidType);

}
