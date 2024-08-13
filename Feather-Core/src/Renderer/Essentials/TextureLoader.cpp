#include "TextureLoader.h"
#include "Logger/Logger.h"

#include <stb_image.h>
#include <SOIL/SOIL.h>

namespace Feather {

    std::shared_ptr<Texture> TextureLoader::Create(Texture::TextureType type, const std::string& texturePath)
    {
		GLuint id;
		int width, height;

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		switch (type)
		{
		case Feather::Texture::TextureType::PIXEL:
			LoadTexture(texturePath, id, width, height, false);
			break;
		case Feather::Texture::TextureType::BLENDED:
			LoadTexture(texturePath, id, width, height, true);
			break;
		// TODO: Add other texture types as needed
		default:
			assert(false && "The current type is not supported, please use another texture type!");
			return nullptr;
		}

		return std::make_shared<Texture>(id, width, height, type, texturePath);
    }

	std::shared_ptr<Texture> TextureLoader::Create(Texture::TextureType type, int width, int height)
	{
		F_ASSERT(type == Texture::TextureType::FRAMEBUFFER && "Must be framebuffer type");

		if (type != Texture::TextureType::FRAMEBUFFER)
		{
			F_ERROR("Failed to create texture for framebuffer. Incorrect type passed in!");
			return nullptr;
		}

		GLuint id;
		glGenTextures(1, &id);
		LoadFBTexture(id, width, height);

		return std::make_shared<Texture>(id, width, height, type);
	}

	std::shared_ptr<Texture> TextureLoader::CreateFromMemory(const unsigned char* imageData, size_t length, bool blended)
	{
		GLuint id;
		int width, height;

		LoadTextureFromMemory(imageData, length, id, width, height, blended);

		return std::make_shared<Texture>(id, width, height, blended ? Texture::TextureType::BLENDED : Texture::TextureType::PIXEL, "");
	}

    bool TextureLoader::LoadTexture(const std::string& filepath, GLuint& id, int& width, int& height, bool blended)
    {
		int channels = 0;
		stbi_uc* image = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

		if (!image)
		{
			F_ERROR("Failed to load image '{0}'", filepath);
			return false;
		}

		GLint format = GL_RGBA;
		switch (channels)
		{
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (!blended)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);

		stbi_image_free(image);

        return true;
    }

	bool TextureLoader::LoadFBTexture(GLuint& id, int& width, int& height)
	{
		glBindTexture(GL_TEXTURE_2D, id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		return true;
	}

	bool TextureLoader::LoadTextureFromMemory(const unsigned char* imageData, size_t length, GLuint& id, int& width, int& height, bool blended)
	{
		id = SOIL_load_OGL_texture_from_memory(imageData, length, SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, NULL);
		if (id == 0)
		{
			F_ERROR("Failed to load texture from memory!");
			return false;
		}

		glBindTexture(GL_TEXTURE_2D, id);
		glad_glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &width);
		glad_glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (!blended)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		return true;
	}

}
