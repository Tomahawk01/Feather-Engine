#pragma once
#include "IDisplay.h"

#include <string>

namespace Feather {

	enum class AssetType;

	class AssetDisplay : public IDisplay
	{
	public:
		AssetDisplay();
		~AssetDisplay() = default;

		virtual void Draw() override;
		virtual void Update() override;

	private:
		void SetAssetType();
		void DrawSelectedAssets();
		unsigned int GetTextureID(const std::string& assetName);
		bool DoRenameAsset(const std::string& oldName, const std::string& newName);
		void CheckRename(const std::string& checkName);

	private:
		const std::vector<std::string> m_SelectableTypes{ "Textures", "Fonts", "Music", "SoundFX", "Scenes" };
		bool m_ItemHovered, m_AssetTypeChanged, m_Rename;
		bool m_WindowSelected, m_WindowHovered;
		std::string m_SelectedAssetName, m_SelectedType, m_DragSource, m_RenameBuf;
		AssetType m_eSelectedType;
		float m_AssetSize;
		int m_SelectedID;
	};

}