#include "Camera2D.h"

namespace Feather {

	Feather::Camera2D::Camera2D()
		: Camera2D(640, 480)
	{}

	Feather::Camera2D::Camera2D(int width, int height)
		: m_Width{ width }, m_Height{ height }, m_Scale{ 1.0f }, m_Position{ 0.0f }, m_ScreenOffset{ 0.0f }, m_CameraMatrix{ 1.0f }, m_OrthoProjection{ 1.0f }, m_NeedUpdate{ true }
	{
		m_OrthoProjection = glm::ortho(0.0f, static_cast<float>(m_Width), static_cast<float>(m_Height), 0.0f, 0.0f, 1.0f);
		Update();
	}

	void Camera2D::Update()
	{
		if (!m_NeedUpdate)
			return;

		// Translate
		glm::vec3 translate{ -m_Position.x, -m_Position.y, 0.0f };
		m_CameraMatrix = glm::translate(m_OrthoProjection, translate);

		// Scale
		glm::vec3 scale{ m_Scale, m_Scale, 0.0f };
		m_CameraMatrix *= glm::scale(glm::mat4(1.0f), scale);

		m_NeedUpdate = false;
	}

	glm::vec2 Camera2D::ScreenCoordsToWorld(const glm::vec2& screenCoords)
	{
		glm::vec2 worldCoords{ screenCoords };

		// Set coords to the center of the screen, scale them and translate the camera
		worldCoords -= m_ScreenOffset;
		worldCoords /= m_Scale;
		worldCoords += m_Position;

		return worldCoords;
	}

	glm::vec2 Camera2D::WorldCoordsToScreen(const glm::vec2& worldCoords)
	{
		glm::vec2 screenCoords{ worldCoords };

		screenCoords += m_ScreenOffset;
		screenCoords *= m_Scale;
		screenCoords -= m_Position;

		return screenCoords;
	}

}
