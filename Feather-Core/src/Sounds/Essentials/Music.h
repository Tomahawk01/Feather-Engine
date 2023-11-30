#pragma once

#include "SoundParams.h"
#include "Utils/SDL_Wrappers.h"

namespace Feather {

	class Music
	{
	public:
		Music(const SoundParams& params, MusicPtr pMusic);
		~Music() = default;

		inline const std::string& GetName() const { return m_Params.name; }
		inline const std::string& GetFilename() const { return m_Params.filename; }
		inline const std::string& GetDescription() const { return m_Params.description; }
		inline const double GetDuration() const { return m_Params.duration; }

		inline Mix_Music* GetMusicPtr() const { if (!m_Music) return nullptr; return m_Music.get(); }

	private:
		SoundParams m_Params{};
		MusicPtr m_Music{ nullptr };
	};

}
