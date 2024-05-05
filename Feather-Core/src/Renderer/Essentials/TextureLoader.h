#pragma once

#include "Texture.h"

#include <memory>

namespace Feather {

	class TextureLoader
	{
	public:
		TextureLoader() = delete;
		static std::shared_ptr<Texture> Create(Texture::TextureType type, const std::string& texturePath);
		static std::shared_ptr<Texture> Create(Texture::TextureType type, int width, int height);

		static std::shared_ptr<Texture> CreateFromMemory(const unsigned char* imageData, size_t length, bool blended = false);

	private:
		static bool LoadTexture(const std::string& filepath, GLuint& id, int& width, int& height, bool blended = false);
		static bool LoadFBTexture(GLuint& id, int& width, int& height);
	};

}
