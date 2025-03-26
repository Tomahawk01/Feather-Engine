#pragma once

#include "Core/ECS/Components/AllComponents.h"
#include "Renderer/Core/Camera2D.h"

namespace Feather {

	bool EntityInView(const TransformComponent& transform, float width, float height, const Camera2D& camera);
	// Transform, Rotation, Scale
	glm::mat4 TRSModel(const TransformComponent& transform, float width, float height);

	void GenerateUVs(SpriteComponent& sprite, int textureWidth, int textureHeight);
	void GenerateUVsExt(SpriteComponent& sprite, int textureWidth, int textureHeight, float u, float v);

}
