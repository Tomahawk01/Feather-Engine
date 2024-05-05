#include "Framebuffer.h"

#include "Logger/Logger.h"
#include "Renderer/Essentials/TextureLoader.h"

namespace Feather {

	Framebuffer::Framebuffer()
		: Framebuffer(640, 480, false)
	{}

	Framebuffer::Framebuffer(int width, int height, bool useRbo)
		: m_FboID{ 0 }, m_RboID{ 0 }, m_Texture{ nullptr },
		m_Width{ width }, m_Height{ height },
		m_ShouldResize{ false }, m_UseRbo{ useRbo }
	{
		m_Texture = std::move(TextureLoader::Create(Texture::TextureType::FRAMEBUFFER, width, height));

		if (!m_Texture || !Initialize())
		{
			F_ASSERT(false && "Failed to create framebuffer");
			F_FATAL("Framebuffer creation failed");
		}
	}

	Framebuffer::~Framebuffer()
	{
		CleanUp();
	}

	void Framebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FboID);
	}

	void Framebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::Resize(int width, int height)
	{
		m_Width = width;
		m_Height = height;
		m_ShouldResize = true;
	}

	void Framebuffer::CheckResize()
	{
		if (!m_ShouldResize)
			return;

		CleanUp();

		m_Texture.reset();
		m_Texture = std::move(TextureLoader::Create(Texture::TextureType::FRAMEBUFFER, m_Width, m_Height));

		F_ASSERT(m_Texture && "New texture cannot be nullptr");

		Initialize();
		m_ShouldResize = false;
	}

	bool Framebuffer::Initialize()
	{
		glCreateFramebuffers(1, &m_FboID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FboID);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture->GetID(), 0);

		if (m_UseRbo)
		{
			glCreateRenderbuffers(1, &m_RboID);
			glBindRenderbuffer(GL_RENDERBUFFER, m_RboID);

			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, m_Width, m_Height);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RboID);
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			F_ASSERT(false && "Failed to create an OpenGL framebuffer");

			std::string error = std::to_string(glGetError());
			F_ERROR("Failed to create an OpenGL framebuffer: {}", error);
			return false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return true;
	}

	void Framebuffer::CleanUp()
	{
		glDeleteFramebuffers(1, &m_FboID);
		if (m_UseRbo)
			glDeleteRenderbuffers(1, &m_RboID);

		if (m_Texture)
		{
			auto textureID = m_Texture->GetID();
			glDeleteTextures(1, &textureID);
		}
	}
}
