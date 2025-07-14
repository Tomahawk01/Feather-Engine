#include "Texture.h"

namespace Feather {

	Texture::Texture()
		: Texture(0, 0, 0, TextureType::NONE, "", false)
	{}

	Texture::Texture(GLuint id, int width, int height, TextureType type, const std::string& texturePath, bool isTileset)
		: m_TextureID{ id }, m_Width{ width }, m_Height{ height }, m_Type{ type }, m_Path{ texturePath }, m_IsTileset{ isTileset }, m_IsEditorTexture{ false }
	{}

	void Feather::Texture::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_TextureID);
	}

	void Feather::Texture::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::Destroy()
	{
		glDeleteTextures(1, &m_TextureID);
	}

}
