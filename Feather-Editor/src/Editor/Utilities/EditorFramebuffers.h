#pragma once
#include "Renderer/Buffers/Framebuffer.h"

#include <map>

namespace Feather {

	enum class FramebufferType
	{
		TILEMAP,
		SCENE,

		NO_TYPE
	};

	struct EditorFramebuffers
	{
		std::map<FramebufferType, std::shared_ptr<Framebuffer>> mapFramebuffers;
	};

}
