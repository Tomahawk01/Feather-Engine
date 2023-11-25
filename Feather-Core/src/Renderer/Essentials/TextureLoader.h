#pragma once

#include "Texture.h"

#include <memory>

namespace Feather {

	class TextureLoader
	{
	public:
		TextureLoader() = delete;
		static std::shared_ptr<Texture> Create(Texture::TextureType type, const std::string& texturePath);

	private:
		static bool LoadTexture(const std::string& filepath, GLuint& id, int& width, int& height, bool blended = false);
	};

}
