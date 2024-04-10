#include "CoreUtilities.h"
#include "Core/ECS/Components/AllComponents.h"

namespace Feather {

    bool EntityInView(const TransformComponent& transform, float width, float height, const Camera2D& camera)
    {
		const auto& cameraPos = camera.GetPosition();
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
		glm::mat4 model{ 1.f };

		if (transform.rotation > 0.f || transform.rotation < 0.f ||
			transform.scale.x > 1.f  || transform.scale.x < 1.f  ||
			transform.scale.y > 1.f  || transform.scale.y < 1.f)
		{
			model = glm::translate(model, glm::vec3{ transform.position, 0.f });
			model = glm::translate(model, glm::vec3{ (width * transform.scale.x) * 0.5f, (height * transform.scale.y) * 0.5f, 0.f });
			model = glm::rotate(model, glm::radians(transform.rotation), glm::vec3{ 0.f, 0.f, 1.f });
			model = glm::translate(model, glm::vec3{ (width * transform.scale.x) * -0.5f, (height * transform.scale.y) * -0.5f, 0.f });
			model = glm::scale(model, glm::vec3{ transform.scale, 1.0f });
			model = glm::translate(model, glm::vec3{ -transform.position, 0.f });
		}

		return model;
    }

}
