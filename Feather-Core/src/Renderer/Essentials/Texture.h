#pragma once

#include <glad/glad.h>

#include <string>

namespace Feather {

	class Texture
	{
	public:
		enum class TextureType
		{
			PIXEL,
			BLENDED,
			NONE
		};

		Texture();
		Texture(GLuint id, int width, int height, TextureType type = TextureType::PIXEL, const std::string& texturePath = "");
		~Texture() = default;

		inline const int GetWidth() const { return m_Width; }
		inline const int GetHeight() const { return m_Height; }
		inline const GLuint GetID() const { return m_TextureID; }

		void Bind();
		void Unbind();

	private:
		GLuint m_TextureID;
		int m_Width, m_Height;
		std::string m_Path;
		TextureType m_Type;
	};

}
