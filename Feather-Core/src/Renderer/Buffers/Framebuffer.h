#pragma once

#include <glad/glad.h>
#include <memory>

#include "Renderer/Essentials/Texture.h"

namespace Feather {

	class Framebuffer
	{
	public:
		Framebuffer();
		Framebuffer(int width, int height, bool useRbo);
		~Framebuffer();

		void Bind();
		void Unbind();

		void Resize(int width, int height);
		void CheckResize();

		inline const GLuint GetID() const { return m_FboID; }
		inline const GLuint GetTextureID() const { return m_Texture ? m_Texture->GetID() : 0; }
		inline const int GetWidth() const { return m_Width; }
		inline const int GetHeight() const { return m_Height; }

	private:
		bool Initialize();
		void CleanUp();

	private:
		GLuint m_FboID;
		GLuint m_RboID;
		std::shared_ptr<Texture> m_Texture;
		int m_Width;
		int m_Height;
		bool m_ShouldResize;
		bool m_UseRbo;
	};
}