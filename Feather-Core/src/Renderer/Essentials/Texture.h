#pragma once

#include <glad/glad.h>

#include <string>

namespace Feather {

	class Texture
	{
	public:
		enum class TextureType
		{
			PIXEL = 0,
			BLENDED,
			FRAMEBUFFER,
			ICON,

			NONE
		};

		Texture();
		Texture(GLuint id, int width, int height, TextureType type = TextureType::PIXEL, const std::string& texturePath = "", bool isTileset = false);
		~Texture() = default;

		inline const int GetWidth() const { return m_Width; }
		inline const int GetHeight() const { return m_Height; }
		inline const GLuint GetID() const { return m_TextureID; }

		inline const bool IsTileset() const { return m_IsTileset; }
		inline TextureType GetType() const { return m_Type; }
		inline void SetIsTileset(bool isTileset) { m_IsTileset = isTileset; }
		inline const std::string& GetPath() const { return m_Path; }
		inline const bool IsEditorTexture() const { return m_IsEditorTexture; }
		inline void SetIsEditorTexture(bool isEditorTexture) { m_IsEditorTexture = isEditorTexture; }

		void Bind();
		void Unbind();

		void Destroy();

	private:
		GLuint m_TextureID;
		int m_Width;
		int m_Height;
		std::string m_Path;
		TextureType m_Type;
		bool m_IsTileset;
		bool m_IsEditorTexture;
	};

}
