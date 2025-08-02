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
		unsigned int GetTextureID(const std::string& assetName) const;
		bool DoRenameAsset(const std::string& oldName, const std::string& newName) const;
		void CheckRename(const std::string& checkName) const;
		void OpenAssetContext(const std::string& assetName);

	protected:
		virtual void DrawToolbar() override;

	private:
		const std::vector<std::string> m_SelectableTypes{ "Textures", "Fonts", "Music", "SoundFX", "Scenes", "Prefabs" };
		bool m_AssetTypeChanged;
		bool m_Rename;
		bool m_WindowSelected;
		bool m_WindowHovered;
		bool m_OpenAddAssetModal;
		std::string m_SelectedAssetName;
		std::string m_SelectedType;
		std::string m_DragSource;
		std::string m_RenameBuf;
		AssetType m_eSelectedType;
		float m_AssetSize;
		int m_SelectedID;
	};

}