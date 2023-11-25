#include "Texture.h"

namespace Feather {

	Texture::Texture()
		: Texture(0, 0, 0, TextureType::NONE)
	{}

	Texture::Texture(GLuint id, int width, int height, TextureType type, const std::string& texturePath)
		: m_TextureID{ id }, m_Width{ width }, m_Height{ height }, m_Type{ type }, m_Path{ texturePath }
	{}

	void Feather::Texture::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_TextureID);
	}

	void Feather::Texture::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}
