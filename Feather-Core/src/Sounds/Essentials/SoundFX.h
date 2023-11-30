#pragma once

#include "SoundParams.h"
#include "Utils/SDL_Wrappers.h"

namespace Feather {

	class SoundFX
	{
	public:
		SoundFX(const SoundParams& params, SoundFXPtr pSoundFx);
		~SoundFX() = default;

		inline const std::string& GetName() const { return m_Params.name; }
		inline const std::string& GetDescription() const { return m_Params.description; }
		inline const std::string& GetFilename() const { return m_Params.filename; }
		inline const double GetDuration() const { return m_Params.duration; }
		inline Mix_Chunk* GetSoundFxPtr() const { if (!m_SoundFx) return nullptr; return m_SoundFx.get(); }

	private:
		SoundParams m_Params;
		SoundFXPtr m_SoundFx;
	};

}
