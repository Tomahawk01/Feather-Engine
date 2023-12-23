#include "Renderer.h"

#include "../Essentials/Shader.h"
#include "Camera2D.h"

namespace Feather {

	Renderer::Renderer()
		: m_LineBatch{ nullptr }, m_SpriteBatch{ nullptr }
	{
		m_LineBatch = std::make_unique<LineBatchRenderer>();
		m_SpriteBatch = std::make_unique<SpriteBatchRenderer>();
	}

	void Renderer::SetClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
	{
		glClearColor(red, green, blue, alpha);
	}

	void Renderer::ClearBuffers(bool color, bool depth, bool stencil)
	{
		glClear(
			(color ? GL_COLOR_BUFFER_BIT : 0) |
			(depth ? GL_DEPTH_BUFFER_BIT : 0) |
			(stencil ? GL_STENCIL_BUFFER_BIT : 0)
		);
	}

	void Renderer::SetCapability(GLCapability capability, bool enable)
	{
		if (enable)
			glEnable(static_cast<GLenum>(capability));
		else
			glDisable(static_cast<GLenum>(capability));
	}

	bool Renderer::IsCapabilityEnabled(GLCapability capability) const
	{
		return glIsEnabled(static_cast<GLenum>(capability));
	}

	void Renderer::SetBlendCapability(BlendingFactors sFactor, BlendingFactors dFactor)
	{
		glBlendFunc(
			static_cast<GLenum>(sFactor),
			static_cast<GLenum>(dFactor)
		);
	}

	void Renderer::SetViewport(GLint x, GLint y, GLsizei width, GLsizei height)
	{
		glViewport(x, y, width, height);
	}

	void Renderer::DrawLine(const Line& line)
	{
		m_Lines.push_back(line);
	}

	void Renderer::DrawLine(const glm::vec2& p1, const glm::vec2& p2, const Color& color, float lineWidth)
	{
		m_Lines.push_back(Line{ .p1 = p1, .p2 = p2, .lineWidth = lineWidth, .color = color });
	}

	void Renderer::DrawRect(const Rect& rect)
	{
		// Top line
		DrawLine(Line{
					.p1 = rect.position,
					.p2 = glm::vec2{rect.position.x + rect.width, rect.position.y},
					.color = rect.color });
		// Bottom line
		DrawLine(Line{
					.p1 = glm::vec2{rect.position.x, rect.position.y + rect.height},
					.p2 = glm::vec2{rect.position.x + rect.width, rect.position.y + rect.height},
					.color = rect.color });
		// Left line
		DrawLine(Line{
					.p1 = rect.position,
					.p2 = glm::vec2{rect.position.x, rect.position.y + rect.height},
					.color = rect.color });
		// Right line
		DrawLine(Line{
					.p1 = glm::vec2{rect.position.x + rect.width, rect.position.y},
					.p2 = glm::vec2{rect.position.x + rect.width, rect.position.y + rect.height},
					.color = rect.color });
	}

	void Renderer::DrawRect(const glm::vec2& position, float width, float height, const Color& color)
	{
		DrawRect(Rect{ .position = position, .width = width, .height = height, .color = color });
	}

	void Renderer::DrawFilledRect(const Rect& rect)
	{
	}

	void Renderer::DrawCircle(const Circle& circle)
	{
	}

	void Renderer::DrawCircle(const glm::vec2& position, float radius, Color& color, float thickness)
	{
	}

	void Renderer::DrawLines(Shader& shader, Camera2D& camera)
	{
		auto cam_mat = camera.GetCameraMatrix();
		shader.Enable();
		shader.SetUniformMat4("projection", cam_mat);

		m_LineBatch->Begin();
		for (const auto& line : m_Lines)
			m_LineBatch->AddLine(line);
		m_LineBatch->End();

		m_LineBatch->Render();
		shader.Disable();
	}

	void Renderer::DrawFilledRects(Shader& shader, Camera2D& camera)
	{
	}

	void Renderer::DrawCircles(Shader& shader, Camera2D& camera)
	{
	}

	void Renderer::ClearPrimitives()
	{
		m_Lines.clear();
		m_Rects.clear();
		m_Circles.clear();
	}

}
