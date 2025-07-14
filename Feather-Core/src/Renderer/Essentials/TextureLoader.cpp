#include "TextureLoader.h"

#include "Logger/Logger.h"
#include "Renderer/Essentials/IconInfo.h"

#include <SOIL/SOIL.h>

#include <fstream>

namespace Feather {

    std::shared_ptr<Texture> TextureLoader::Create(Texture::TextureType type, const std::string& texturePath, bool isTileset)
    {
		GLuint id;
		int width, height;

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		bool loadSuccessful{ false };
		switch (type)
		{
		case Texture::TextureType::PIXEL:
			loadSuccessful = LoadTexture(texturePath, id, width, height, false);
			break;
		case Texture::TextureType::BLENDED:
			loadSuccessful = LoadTexture(texturePath, id, width, height, true);
			break;
		case Texture::TextureType::ICON:
			loadSuccessful = LoadIconTexture(texturePath, id, width, height);
			break;
		default:
			assert(false && "The current type is not supported, please use another texture type!");
			return nullptr;
		}

		return loadSuccessful ? std::make_shared<Texture>(id, width, height, type, texturePath, isTileset) : nullptr;
    }

	std::shared_ptr<Texture> TextureLoader::Create(Texture::TextureType type, int width, int height, bool isTileset)
	{
		F_ASSERT(type == Texture::TextureType::FRAMEBUFFER && "Must be framebuffer type");

		if (type != Texture::TextureType::FRAMEBUFFER)
		{
			F_ERROR("Failed to create texture for framebuffer. Incorrect type passed in!");
			return nullptr;
		}

		GLuint id;
		glGenTextures(1, &id);
		if (LoadFBTexture(id, width, height))
		{
			return std::make_shared<Texture>(id, width, height, type, "", isTileset);
		}

		return nullptr;
	}

	std::shared_ptr<Texture> TextureLoader::CreateFromMemory(const unsigned char* imageData, size_t length, bool blended, bool isTileset)
	{
		GLuint id;
		int width, height;

		if (LoadTextureFromMemory(imageData, length, id, width, height, blended))
		{
			return std::make_shared<Texture>(id, width, height, blended ? Texture::TextureType::BLENDED : Texture::TextureType::PIXEL, "", isTileset);
		}

		return nullptr;
	}

    bool TextureLoader::LoadTexture(const std::string& filepath, GLuint& id, int& width, int& height, bool blended)
    {
		int channels = 0;
		unsigned char* image = SOIL_load_image(filepath.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);

		if (!image)
		{
			F_ERROR("Failed to load image '{0}': {1}", filepath, SOIL_last_result());
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

		SOIL_free_image_data(image);

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

	bool TextureLoader::LoadIconTexture(const std::string& filepath, GLuint& id, int& width, int& height)
	{
		std::ifstream file(filepath, std::ios::binary | std::ios::ate);

		if (!file)
		{
			F_ERROR("Failed to open ICO file: {}", filepath);
			return false;
		}

		const std::streamsize fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		std::vector<uint8_t> fileData(fileSize);
		file.read(reinterpret_cast<char*>(fileData.data()), fileSize);

		ICONDIR* iconDir = reinterpret_cast<ICONDIR*>(fileData.data());
		if (iconDir->type != 1 || iconDir->count == 0)
		{
			F_ERROR("Failed to load Icon Texture '{}': Invalid ICO format", filepath);
			return false;
		}

		ICONDIRENTRY* entry = reinterpret_cast<ICONDIRENTRY*>(fileData.data() + sizeof(ICONDIR));

		size_t offset = entry->imageOffset;
		size_t size = entry->bytesInRes;

		if (offset + size > fileData.size())
		{
			F_ERROR("Corrupted ICO Image");
			return false;
		}

		uint8_t* imageData = fileData.data() + offset;

		if (IsPNG(imageData, size))
		{
			width = 0;
			height = 0;

			id = SOIL_load_OGL_texture_from_memory(imageData, static_cast<int>(size), SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, NULL);
			if (id == 0)
			{
				F_ERROR("Failed to load icon. Failed to decode PNG inside of ICO");
				return false;
			}

			// Get the width and height data from the texture
			glBindTexture(GL_TEXTURE_2D, id);
			glad_glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &width);
			glad_glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &height);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else
		{
			BITMAPINFOHEADER* bmpHeader = reinterpret_cast<BITMAPINFOHEADER*>(imageData);

			if (bmpHeader->bitCount != 32)
			{
				F_ERROR("Only 32-bit BMP icons or PNG icons are supported");
				return false;
			}

			width = bmpHeader->width;
			height = bmpHeader->height / 2; // Height includes XOR + AND MASK

			size_t pixelDataSize = width * height * 4;
			uint8_t* pixelData = imageData + bmpHeader->size;

			// Convert BGRA -> RGBA and flip vertically
			std::vector<uint8_t> rgbaData(pixelDataSize);
			for (int y = 0; y < height; ++y)
			{
				for (int x = 0; x < width; ++x)
				{
					int srcIndex = ((height - 1 - y) * width + x) * 4;
					int dstIndex = (y * width + x) * 4;

					rgbaData[dstIndex + 0] = pixelData[srcIndex + 2];
					rgbaData[dstIndex + 1] = pixelData[srcIndex + 1];
					rgbaData[dstIndex + 2] = pixelData[srcIndex + 0];
					rgbaData[dstIndex + 3] = pixelData[srcIndex + 3];
				}
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData.data());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		return true;
	}

	bool TextureLoader::IsPNG(const uint8_t* data, size_t size)
	{
		static const uint8_t pngSig[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };
		return size >= 8 && std::memcmp(data, pngSig, 8) == 0;
	}

}
