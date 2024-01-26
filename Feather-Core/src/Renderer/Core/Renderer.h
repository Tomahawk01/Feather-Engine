#pragma once

#include "../Essentials/Primitives.h"
#include "BatchRenderer.h"
#include "LineBatchRenderer.h"
#include "RectBatchRenderer.h"
#include "CircleBatchRenderer.h"
#include "TextBatchRenderer.h"

#include <glad/glad.h>

#include <memory>
#include <vector>

namespace Feather {

	class Renderer
	{
	public:
		enum class GLCapability : GLuint
		{
			BLEND = GL_BLEND,
			DEPTH_TEST = GL_DEPTH_TEST,
			LINE_SMOOTH = GL_LINE_SMOOTH
		};

		enum class BlendingFactors : GLuint
		{
			ONE = GL_ONE,
			SRC_ALPHA = GL_SRC_ALPHA,
			ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA
		};

	public:
		Renderer();
		~Renderer() = default;

		void SetClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		void ClearBuffers(bool color = true, bool depth = true, bool stencil = false);

		void SetCapability(GLCapability capability, bool enable);
		bool IsCapabilityEnabled(GLCapability capability) const;

		void SetBlendCapability(BlendingFactors sFactor, BlendingFactors dFactor);
		void SetViewport(GLint x, GLint y, GLsizei width, GLsizei height);

		void SetLineWidth(GLfloat lineWidth);

		void DrawLine(const Line& line);
		void DrawLine(const glm::vec2& p1, const glm::vec2& p2, const Color& color, float lineWidth = 1.0f);

		void DrawRect(const Rect& rect);
		void DrawRect(const glm::vec2& position, float width, float height, const Color& color);
		void DrawFilledRect(const Rect& rect);

		void DrawCircle(const Circle& circle);
		void DrawCircle(const glm::vec2& position, float radius, const Color& color, float thickness = 1.0f);

		void DrawText2D(const Text& text);

		void DrawLines(class Shader& shader, class Camera2D& camera);
		void DrawFilledRects(class Shader& shader, class Camera2D& camera);
		void DrawCircles(class Shader& shader, class Camera2D& camera);
		void DrawAllText(class Shader& shader, class Camera2D& camera);

		void ClearPrimitives();

	private:
		std::vector<Line> m_Lines;
		std::vector<Rect> m_Rects;
		std::vector<Circle> m_Circles;
		std::vector<Text> m_Text;

		std::unique_ptr<LineBatchRenderer> m_LineBatch;
		std::unique_ptr<RectBatchRenderer> m_RectBatch;
		std::unique_ptr<CircleBatchRenderer> m_CircleBatch;
		std::unique_ptr<SpriteBatchRenderer> m_SpriteBatch;
		std::unique_ptr<TextBatchRenderer> m_TextBatch;
	};

}
