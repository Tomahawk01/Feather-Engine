#include "AssetDisplayUtils.h"

#include "Logger/Logger.h"
#include "Utils/FeatherUtilities.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Filesystem/Dialogs/FileDialog.h"
#include "Editor/scene/SceneManager.h"

#include <imgui.h>
#include <filesystem>

#define IMAGE_FILTERS                                                                                                  \
	std::vector<const char*>                                                                                           \
	{                                                                                                                  \
		"*.png", "*.bmp", "*.jpg"                                                                                      \
	}
#define FONT_FILTERS                                                                                                   \
	std::vector<const char*>                                                                                           \
	{                                                                                                                  \
		"*.ttf" /* add more font types */                                                                              \
	}
#define MUSIC_FILTERS                                                                                                  \
	std::vector<const char*>                                                                                           \
	{                                                                                                                  \
		"*.mp3", "*.wav", "*.ogg"                                                                                      \
	}
#define SOUNDFX_FILTERS                                                                                                \
	std::vector<const char*>                                                                                           \
	{                                                                                                                  \
		"*.mp3", "*.wav", "*.ogg"                                                                                      \
	}

namespace {

	class AssetModalCreator
	{
	public:
		AssetModalCreator() {}

		bool AddAssetBasedOnType(const std::string& assetName, const std::string& filepath, Feather::AssetType assetType, bool isPixelArt = true, bool isTileset = false, float fontSize = 32.0f)
		{
			auto& assetManager = MAIN_REGISTRY().GetAssetManager();
			switch (assetType)
			{
			case Feather::AssetType::TEXTURE: return assetManager.AddTexture(assetName, filepath, isPixelArt, isTileset);
			case Feather::AssetType::FONT: return assetManager.AddFont(assetName, filepath, fontSize);
			case Feather::AssetType::SOUNDFX: return assetManager.AddSoundFx(assetName, filepath);
			case Feather::AssetType::MUSIC: return assetManager.AddMusic(assetName, filepath);
			case Feather::AssetType::SCENE: return false;
			}
			return false;
		}

		std::string CheckForAsset(const std::string& assetName, Feather::AssetType assetType)
		{
			std::string error{ "" };
			if (assetName.empty())
			{
				error = "Asset name cannot be empty!";
			}
			else if (assetType == Feather::AssetType::SCENE)
			{
				if (SCENE_MANAGER().HasScene(assetName))
					error = std::format("Scene '{}' already exists!", assetName);
			}
			else
			{
				if (MAIN_REGISTRY().GetAssetManager().HasAsset(assetName, assetType))
					error = std::format("Asset '{}' already exists!", assetName);
			}

			return error;
		}

		void AddSceneModal(bool* pbOpen)
		{
			if (*pbOpen)
				ImGui::OpenPopup("Add New Scene");

			if (ImGui::BeginPopupModal("Add New Scene"))
			{
				static std::string assetName{ "" };
				ImGui::InputText("##assetName", assetName.data(), 255);

				std::string checkName{ assetName.data() };
				std::string nameError{ CheckForAsset(checkName, Feather::AssetType::SCENE) };

				if (nameError.empty())
				{
					if (ImGui::Button("Ok"))
					{
						if (!SCENE_MANAGER().AddScene(checkName))
							F_ERROR("Failed to add new scene '{}'", checkName);

						assetName.clear();

						*pbOpen = false;
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
				}
				else
				{
					ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, nameError.c_str());
				}

				// We always want to be able to cancel
				if (ImGui::Button("Cancel"))
				{
					assetName.clear();
					*pbOpen = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		void AddAssetModal(Feather::AssetType assetType, bool* pbOpen)
		{
			std::string sAssetType{ Feather::AssetDisplayUtils::AddAssetBasedOnType(assetType) };

			if (*pbOpen)
				ImGui::OpenPopup(sAssetType.c_str());

			if (ImGui::BeginPopupModal(sAssetType.c_str()))
			{
				static std::string assetName{ "" };
				static std::string filepath{ "" };
				static bool isTileset{ false };
				static bool isPixelArt{ false };
				static float fontSize{ 32.0f };

				std::string checkName{ assetName.data() };
				std::string sCheckFilepath{ filepath.data() };

				ImGui::InputText("##assetName", assetName.data(), 255);
				std::string nameError{ CheckForAsset(checkName, assetType) };
				ImGui::InputText("##filepath", filepath.data(), 255);
				ImGui::SameLine();
				if (ImGui::Button("Browse"))
				{
					Feather::FileDialog fd{};
					filepath = fd.OpenFileDialog("Open", "", Feather::AssetDisplayUtils::GetAssetFileFilters(assetType));

					if (!filepath.empty())
					{
						std::filesystem::path path{ filepath };
						assetName = path.stem().string();
					}
				}

				if (assetType == Feather::AssetType::TEXTURE)
				{
					ImGui::Checkbox("Pixel Art", &isPixelArt);
					ImGui::Checkbox("Tileset", &isTileset);
				}
				else if (assetType == Feather::AssetType::FONT)
				{
					ImGui::InputFloat("Font Size", &fontSize, 1.0f, 1.0f, "%.1f");
				}

				if (nameError.empty())
				{
					if (ImGui::Button("Ok"))
					{
						if (std::filesystem::exists(std::filesystem::path{ sCheckFilepath }))
						{
							if (!AddAssetBasedOnType(checkName, sCheckFilepath, assetType, isPixelArt, isTileset, fontSize))
								F_ERROR("Failed to add new texture!");

							isTileset = false;
							isPixelArt = false;
							assetName.clear();
							filepath.clear();
							*pbOpen = false;
							ImGui::CloseCurrentPopup();
						}
						else
						{
							// TODO: Add filepath error
						}
					}
					ImGui::SameLine();
				}
				else
				{
					ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, nameError.c_str());
				}

				// We always want to be able to cancel
				if (ImGui::Button("Cancel"))
				{
					isTileset = false;
					isPixelArt = false;
					assetName.clear();
					filepath.clear();
					*pbOpen = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
	};

}

namespace Feather {

    std::vector<const char*> AssetDisplayUtils::GetAssetFileFilters(AssetType assetType)
    {
		switch (assetType)
		{
		case AssetType::TEXTURE: return IMAGE_FILTERS;
		case AssetType::FONT: return FONT_FILTERS;
		case AssetType::SOUNDFX: return SOUNDFX_FILTERS;
		case AssetType::MUSIC: return MUSIC_FILTERS;
		}

		return {};
    }

    std::string AssetDisplayUtils::AddAssetBasedOnType(AssetType assetType)
    {
		switch (assetType)
		{
		case AssetType::TEXTURE: return "Add Texture";
		case AssetType::FONT: return "Add Font";
		case AssetType::SOUNDFX: return "Add SoundFx";
		case AssetType::MUSIC: return "Add Music";
		case AssetType::SCENE: return "Add Scene";
		default: F_ASSERT(false && "Type has not been implemented!"); return {};
		}
    }

    void AssetDisplayUtils::OpenAddAssetModalBasedOnType(AssetType assetType, bool* isOpen)
    {
		F_ASSERT(assetType != AssetType::NO_TYPE && "The asset type must be set!");
		static AssetModalCreator md{};
		if (assetType == AssetType::SCENE)
		{
			md.AddSceneModal(isOpen);
		}
		else
		{
			md.AddAssetModal(assetType, isOpen);
		}
    }

}
