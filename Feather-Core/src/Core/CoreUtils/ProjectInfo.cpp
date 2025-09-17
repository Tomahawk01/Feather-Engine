#include "ProjectInfo.h"

#include "Logger/Logger.h"
#include "Renderer/Essentials/Texture.h"
#include "Renderer/Essentials/TextureLoader.h"

#include <SDL_mixer.h>

namespace Feather {

	void ProjectInfo::SetProjectPath(const fs::path& path)
	{
		F_ASSERT(fs::exists(path) && "Project path must exist!");
		m_ProjectPath = path;
	}

	const fs::path& ProjectInfo::GetProjectPath() const
	{
		F_ASSERT(fs::exists(m_ProjectPath) && "Project path was not set correctly!");
		return m_ProjectPath;
	}

	void ProjectInfo::SetProjectFilePath(const fs::path& path)
	{
		F_ASSERT(fs::exists(path) && "Project file path must exist!");
		m_ProjectFilePath = path;
	}

	std::optional<fs::path> ProjectInfo::GetProjectFilePath() const
	{
		return m_ProjectFilePath;
	}

	void ProjectInfo::SetMainLuaScriptPath(const fs::path& path)
	{
		F_ASSERT(fs::exists(path) && "Main Lua script file path must exist!");
		m_MainLuaScript = path;
	}

	std::optional<fs::path> ProjectInfo::GetMainLuaScriptPath() const
	{
		return m_MainLuaScript;
	}

	void ProjectInfo::SetScriptListPath(const fs::path& path)
	{
		F_ASSERT(fs::exists(path) && "Script list file path must exist!");
		m_ScriptListPath = path;
	}

	std::optional<fs::path> ProjectInfo::GetScriptListPath() const
	{
		return m_ScriptListPath;
	}

	void ProjectInfo::SetFileIconPath(const fs::path& path)
	{
		F_ASSERT(fs::exists(path) && "File Icon Path Must Exist!");
		m_IconFilePath = path;

		// Set the texture
		if (m_IconTexture)
		{
			m_IconTexture->Destroy();
			m_IconTexture.reset();
		}

		m_IconTexture = TextureLoader::Create(Texture::TextureType::ICON, path.string(), false);
		if (!m_IconTexture)
		{
			F_ERROR("Failed to load icon texture: '{}'", path.string());
			return;
		}

		//F_ASSERT( m_IconTexture && "Loading of icon texture failed" );
		F_TRACE("Created Icon Texture: '{}'", path.string());
	}

	std::optional<fs::path> ProjectInfo::GetFileIconPath() const
	{
		return m_IconFilePath;
	}

	bool ProjectInfo::AddFolderPath(EProjectFolderType folderType, const fs::path& path)
	{
		auto [it, success] = m_mapProjectFolderPaths.try_emplace(folderType, path);
		return success;
	}

	std::optional<fs::path> ProjectInfo::TryGetFolderPath(EProjectFolderType folderType)
	{
		if (auto it = m_mapProjectFolderPaths.find(folderType); it != m_mapProjectFolderPaths.end())
		{
			return it->second;
		}

		return std::nullopt;
	}

	bool AudioConfigInfo::UpdateSoundChannels(int numChannels)
	{
		if (allocatedSoundChannels + numChannels > 64)
		{
			F_ERROR("Failed to update sound channels. Max 64 channels are supported");
			return false;
		}
		else if (allocatedSoundChannels + numChannels < 8)
		{
			F_ERROR("Failed to update sound channels. There must be at least 8 sound channels");
			return false;
		}

		if (numChannels > 0)
			AddChannels(numChannels);
		else if (numChannels < 0)
			RemoveChannels(-numChannels);

		Mix_AllocateChannels(allocatedSoundChannels);

		return true;
	}

	bool AudioConfigInfo::EnableChannelOverride(int channel, bool enable)
	{
		auto channelItr = mapSoundChannelVolume.find(channel);
		if (channelItr == mapSoundChannelVolume.end())
		{
			F_ERROR("Failed to change sound channel override. Channel '{}' is invalid", channel);
			return false;
		}

		channelItr->second.first = enable;
		return true;
	}

	bool AudioConfigInfo::SetChannelVolume(int channel, int volume)
	{
		auto channelItr = mapSoundChannelVolume.find(channel);
		if (channelItr == mapSoundChannelVolume.end())
		{
			F_ERROR("Failed to set sound channel volume. Channel '{}' is invalid", channel);
			return false;
		}

		channelItr->second.second = volume;
		return true;
	}

	void AudioConfigInfo::AddChannels(int numChannels)
	{
		for (int i = 0; i < numChannels; ++i)
		{
			int channelID{ allocatedSoundChannels + i };
			mapSoundChannelVolume.emplace(channelID, std::make_pair(false, 100));
		}

		allocatedSoundChannels += numChannels;
	}

	void AudioConfigInfo::RemoveChannels(int numChannels)
	{
		for (int i = 0; i < numChannels && allocatedSoundChannels > 0; ++i)
		{
			int channelID{ allocatedSoundChannels - 1 };
			mapSoundChannelVolume.erase(channelID);
			--allocatedSoundChannels;
		}
	}

}
