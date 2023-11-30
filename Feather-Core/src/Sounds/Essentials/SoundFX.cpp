#include "SoundFX.h"

Feather::SoundFX::SoundFX(const SoundParams& params, SoundFXPtr pSoundFx)
	: m_Params{ params }, m_SoundFx{ std::move(pSoundFx) }
{}
