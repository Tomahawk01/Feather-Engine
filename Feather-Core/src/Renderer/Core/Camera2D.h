#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Feather {

	class Camera2D
	{
	public:
		Camera2D();
		Camera2D(int width, int height);
		~Camera2D();

		void Update();
		void Reset();
		void Resize(int newWidth, int newHeight);

		glm::vec2 ScreenCoordsToWorld(const glm::vec2& screenCoords) const;
		glm::vec2 WorldCoordsToScreen(const glm::vec2& worldCoords) const;

		inline void SetPosition(glm::vec2 newPosition) { m_Position = newPosition; m_NeedUpdate = true; }
		inline void SetScreenOffset(glm::vec2 newOffset) { m_ScreenOffset = newOffset; m_NeedUpdate = true; }
		inline void SetScale(float scale)
		{
			// NOTE: Prevent scale from being zero
			if (scale <= 0.0f)
				scale = 0.1f;

			m_Scale = scale;
			m_NeedUpdate = true;
		}

		inline const glm::vec2 GetPosition() const { return m_Position; }
		inline const glm::vec2 GetScreenOffset() const { return m_ScreenOffset; }
		inline const float GetScale() const { return m_Scale; }
		inline const int GetWidth() const { return m_Width; }
		inline const int GetHeight() const { return m_Height; }

		inline glm::mat4 GetCameraMatrix() const { return m_CameraMatrix; }

	private:
		void Initialize();

	private:
		int m_Width;
		int m_Height;
		float m_Scale;

		glm::vec2 m_Position;
		glm::vec2 m_ScreenOffset;

		glm::mat4 m_CameraMatrix;
		glm::mat4 m_OrthoProjection;

		bool m_NeedUpdate;
	};

}
