#include "CoreUtilities.h"
#include "Core/ECS/Components/AllComponents.h"

namespace Feather {

    bool EntityInView(const TransformComponent& transform, float width, float height, const Camera2D& camera)
    {
		const auto& cameraPos = camera.GetPosition() - camera.GetScreenOffset();
		const auto& cameraWidth = camera.GetWidth();
		const auto& cameraHeight = camera.GetHeight();
		const auto& cameraScale = camera.GetScale();

		if ((transform.position.x <= ((cameraPos.x - (width * transform.scale.x * cameraScale)) / cameraScale) ||
			 transform.position.x >= ((cameraPos.x + cameraWidth) / cameraScale)) ||
			(transform.position.y <= ((cameraPos.y - (height * transform.scale.y * cameraScale)) / cameraScale) ||
			 transform.position.y >= ((cameraPos.y + cameraHeight) / cameraScale)))
		{
			return false;
		}

		// Sprite is in view
        return true;
    }

    glm::mat4 TRSModel(const TransformComponent& transform, float width, float height)
    {
		glm::mat4 model{ 1.0f };

		if (transform.rotation > 0.0f || transform.rotation < 0.0f ||
			transform.scale.x > 1.0f  || transform.scale.x < 1.0f  ||
			transform.scale.y > 1.0f  || transform.scale.y < 1.0f)
		{
			model = glm::translate(model, glm::vec3{ transform.position, 0.0f });
			model = glm::translate(model, glm::vec3{ (width * transform.scale.x) * 0.5f, (height * transform.scale.y) * 0.5f, 0.0f });
			model = glm::rotate(model, glm::radians(transform.rotation), glm::vec3{ 0.0f, 0.0f, 1.0f });
			model = glm::translate(model, glm::vec3{ (width * transform.scale.x) * -0.5f, (height * transform.scale.y) * -0.5f, 0.0f });
			model = glm::scale(model, glm::vec3{ transform.scale, 1.0f });
			model = glm::translate(model, glm::vec3{ -transform.position, 0.0f });
		}

		return model;
    }

	void GenerateUVs(SpriteComponent& sprite, int textureWidth, int textureHeight)
	{
		sprite.uvs.uv_width = sprite.width / textureWidth;
		sprite.uvs.uv_height = sprite.height / textureHeight;

		sprite.uvs.u = sprite.start_x * sprite.uvs.uv_width;
		sprite.uvs.v = sprite.start_y * sprite.uvs.uv_height;
	}

	void GenerateUVsExt(SpriteComponent& sprite, int textureWidth, int textureHeight, float u, float v)
	{
		sprite.uvs.uv_width = sprite.width / textureWidth;
		sprite.uvs.uv_height = sprite.height / textureHeight;

		sprite.uvs.u = u;
		sprite.uvs.v = v;
	}

}
