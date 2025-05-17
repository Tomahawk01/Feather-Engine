#include "CoreUtilities.h"

#include "Core/ECS/Components/AllComponents.h"
#include "Core/Scene/Scene.h"
#include "Utils/MathUtilities.h"

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

	std::tuple<int, int> ConvertWorldPosToIsoCoords(const glm::vec2& position, const Canvas& canvas)
	{
		// TODO: Enforce width being double height. Double width hack for now. 
		float doubleWidth = canvas.tileWidth * 2.0f;

		// Move the x position back the halfWidth of one tile.
		// We are not currently using any offset for we have a camera that can go into the negatives.
		float xPos = (position.x /*- canvas.offset.x*/) - doubleWidth * 0.5f;
		// Stretch the Y position by 2 in the negative to make the tile a square. Allows for easier rotations.
		float yPos = position.y * -2.0f;

		// Rotate both the x and the y positions by 45 degrees
		float px = xPos * cos(PIOver4) - yPos * sin(PIOver4);
		float py = xPos * sin(PIOver4) + yPos * cos(PIOver4);

		float diagonal = canvas.tileHeight * sqrt(2.0f);

		int cellX = static_cast<int>(px / diagonal);
		int cellY = static_cast<int>(-py / diagonal);

		return std::make_tuple(cellX, cellY);
	}

}
