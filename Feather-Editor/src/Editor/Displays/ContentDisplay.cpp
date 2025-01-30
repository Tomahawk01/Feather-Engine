#include "ContentDisplay.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Core/Events/EventDispatcher.h"
#include "Utils/FeatherUtilities.h"
#include "FileSystem/Dialogs/FileDialog.h"

#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Utilities/ImGuiUtils.h"
#include "Editor/Utilities/fonts/IconsFontAwesome5.h"
#include "Editor/Events/EditorEventTypes.h"

#include <imgui.h>

namespace Feather {

	ContentDisplay::ContentDisplay()
		: m_FileDispatcher{ std::make_unique<EventDispatcher>() }
		, m_CurrentDir{ DEFAULT_PROJECT_PATH }
		, m_FilepathToAction{ "" }
		, m_Selected{ -1 }
		, m_FileAction{ FileAction::NoAction }
		, m_ItemCut{ false }
		, m_WindowHovered{ false }
	{
		ADD_EVENT_HANDLER(FileEvent, &ContentDisplay::HandleFileEvent, *this);
		m_FileDispatcher->AddHandler<FileEvent, &ContentDisplay::HandleFileEvent>(*this);
	}

	ContentDisplay::~ContentDisplay()
	{}

	void ContentDisplay::Update()
	{
		m_FileDispatcher->UpdateAll();
	}

	void ContentDisplay::Draw()
	{
		if (!ImGui::Begin("Content Browser"))
		{
			ImGui::End();
			return;
		}

		const float& windowWidth = ImGui::GetWindowWidth();
		int numCols = windowWidth / 128;
		if (numCols == 0)
		{
			F_ERROR("NumCols is zero!");
			ImGui::End();
			return;
		}

		DrawToolbar();

		int size = static_cast<int>(std::distance(std::filesystem::directory_iterator(m_CurrentDir), std::filesystem::directory_iterator{}));
		int numRows = size / numCols < 1 ? 1 : (size / numCols) + 1;
		auto itr = std::filesystem::directory_iterator(m_CurrentDir);

		if (ImGui::BeginTable("Content", numCols, IMGUI_NORMAL_TABLE_FLAGS))
		{
			m_WindowHovered = ImGui::IsWindowHovered();
			static ImGuiID popID = 0;

			for (int i = 0; i < numRows; i++)
			{
				ImGui::TableNextRow();
				for (int j = 0; j < numCols; j++)
				{
					if (itr == std::filesystem::directory_iterator{})
						break;

					const auto& path = itr->path();

					auto relativePath = std::filesystem::relative(path, m_CurrentDir);
					auto filenameStr = relativePath.filename().string();

					ImGui::TableSetColumnIndex(j);
					ImGui::PushID(j + j + i);
					int id = j + j + i;

					if (m_Selected == id)
					{
						ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4{ 0.0f, 0.9f, 0.0f, 0.3f }));
					}

					const auto* icon = GetIconTexture(path.string());
					static bool itemPop{ false };

					if (itr->is_directory())
					{
						// Change to the next Directory
						ImGui::ImageButton((ImTextureID)icon->GetID(), ImVec2{ 80.0f, 80.0f });
						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
						{
							m_CurrentDir /= path.filename();
							m_Selected = -1;
						}
						else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
						{
							m_Selected = id;
						}
						else if (!ImGui::IsItemHovered() && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0))
						{
							m_Selected = -1;
						}
					}
					else
					{
						ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 0.0f });
						ImGui::ImageButton((ImTextureID)icon->GetID(), ImVec2{ 80.0f, 80.0f });
						ImGui::PopStyleVar(1);

						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
						{
							// TODO: Open new process to run the file
						}
						else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
						{
							m_Selected = id;
						}
					}

					if (ImGui::BeginPopupContextItem())
					{
						popID = ImGui::GetItemID();
						if (m_ItemCut)
						{
							ImGui::BeginDisabled();
							ImGui::Selectable("Cut");
							ImGui::EndDisabled();
						}
						else
						{
							if (ImGui::Selectable("Cut"))
							{
								m_FilepathToAction = path.string();
								m_ItemCut = true;
							}
							if (ImGui::Selectable("Delete"))
							{
								if (m_Selected == id)
								{
									m_FilepathToAction = path.string();
									m_FileAction = FileAction::Delete;
								}
							}
						}

						itemPop = true;
						ImGui::EndPopup();
					}

					ImGui::SetNextItemWidth(80.0f);
					ImGui::TextWrapped(filenameStr.c_str());

					if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel) && itemPop)
					{
						popID = 0;
						itemPop = false;
					}

					ImGui::PopID();
					++itr;
				}
			}

			if (popID == 0)
			{
				if (ImGui::BeginPopupContextWindow())
				{
					if (m_ItemCut && !m_FilepathToAction.empty())
					{
						if (ImGui::Selectable("Paste"))
						{
							m_FileDispatcher->EnqueueEvent(FileEvent{ .eAction = FileAction::Paste });
						}
					}
					else
					{
						ImGui::BeginDisabled();
						ImGui::Selectable("Paste");
						ImGui::EndDisabled();
					}
					ImGui::EndPopup();
				}
			}

			ImGui::EndTable();
		}

		OpenDeletePopup();

		ImGui::End();
	}

	void ContentDisplay::DrawToolbar()
	{
		ImGui::Separator();

		if (ImGui::Button(ICON_FA_FOLDER_PLUS))
		{
		}
		ImGui::ItemToolTip("Create Folder");
		ImGui::SameLine(0.0f, 16.0f);

		std::string pathStr{ m_CurrentDir.string() };
		std::string pathToSplit = pathStr.substr(pathStr.find(BASE_PATH) + BASE_PATH.size());
		auto dir = SplitStr(pathToSplit, '\\');
		for (size_t i = 0; i < dir.size(); i++)
		{
			if (ImGui::Button(dir[i].c_str()))
			{
				std::string pathChange = pathStr.substr(0, pathStr.find(dir[i]) + dir[i].size());
				m_CurrentDir = std::filesystem::path{ pathChange };
			}

			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, BLACK_TRANSPARENT);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, BLACK_TRANSPARENT);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, BLACK_TRANSPARENT);
			ImGui::Button(ICON_FA_ANGLE_RIGHT, { 16.0f, 18.0f });
			ImGui::PopStyleColor(3);

			if (i < dir.size() - 1)
				ImGui::SameLine();
		}

		ImGui::Separator();
	}

	void ContentDisplay::CopyDroppedFile(const std::string& fileToCopy, const std::filesystem::path& droppedPath)
	{
		if (droppedPath.empty())
			return;

		std::error_code ec;
		if (!std::filesystem::exists(droppedPath, ec))
		{
			F_ERROR("Failed to copy dropped file {}: {}", fileToCopy, ec.message());
			return;
		}

		std::filesystem::path source{ fileToCopy };
		std::filesystem::path destination = std::filesystem::is_directory(droppedPath) ? droppedPath : droppedPath.parent_path();
		destination /= source.filename();

		F_TRACE("Source: {}", fileToCopy);
		F_TRACE("Destination {}", destination.string());

		if (std::filesystem::is_directory(source))
		{
			std::filesystem::copy(source, destination, std::filesystem::copy_options::recursive, ec);
			if (ec)
			{
				F_ERROR("Failed to copy the directory '{}': {}", fileToCopy, ec.message());
			}
		}
		else if (!std::filesystem::copy_file(source, destination, ec))
		{
			F_ERROR("Failed to copy the file '{}': {}", fileToCopy, ec.message());
		}
	}

	void ContentDisplay::MoveFolderOrFile(const std::filesystem::path& movedPath, const std::filesystem::path& path)
	{
		if (!std::filesystem::is_directory(movedPath))
		{
			std::filesystem::rename(movedPath, path / movedPath.filename());
		}
		else
		{
			auto foundFolder = std::filesystem::path{ GET_SUBSTRING(movedPath.string(), std::string{ PATH_SEPARATOR }) };
			try
			{
				if (!std::filesystem::is_empty(movedPath))
				{
					for (const auto& entry : std::filesystem::recursive_directory_iterator(movedPath))
					{
						auto newPath = path / foundFolder;
						if (!std::filesystem::exists(newPath))
							std::filesystem::create_directory(newPath);

						newPath /= entry.path().filename();
						std::filesystem::rename(entry.path(), newPath);
					}
				}
				else
				{
					std::filesystem::rename(movedPath, path / foundFolder);
				}
			}
			catch (const std::filesystem::filesystem_error& error)
			{
				F_ERROR("Failed to move folder and files: {}", error.what());
			}

			// If the directory is empty, remove it
			if (std::filesystem::exists(movedPath) && std::filesystem::is_empty(movedPath))
				std::filesystem::remove(movedPath);
		}
	}

	void ContentDisplay::HandleFileEvent(const FileEvent& fileEvent)
	{
		if (fileEvent.sFilepath.empty() || fileEvent.eAction == FileAction::NoAction)
			return;

		switch (fileEvent.eAction)
		{
		case FileAction::Delete:
		{
			std::filesystem::path path{ fileEvent.sFilepath };
			if (std::filesystem::is_directory(path))
				std::filesystem::remove_all(path);
			else if (std::filesystem::is_regular_file(path))
				std::filesystem::remove(path);
			break;
		}
		case FileAction::Paste:
		{
			if (std::filesystem::is_directory(m_CurrentDir))
			{
				MoveFolderOrFile(m_FilepathToAction, m_CurrentDir);
				m_ItemCut = false;
				m_FilepathToAction.clear();
			}
			break;
		}
		case FileAction::FileDropped:
		{
			if (!m_WindowHovered || fileEvent.sFilepath.empty())
				break;

			CopyDroppedFile(fileEvent.sFilepath, m_CurrentDir);

			F_TRACE("Dropped file: {}", fileEvent.sFilepath);
			break;
		}
		}
	}

	void ContentDisplay::OpenDeletePopup()
	{
		if (m_FileAction != FileAction::Delete)
			return;

		ImGui::OpenPopup("Delete");

		if (ImGui::BeginPopupModal("Delete"))
		{
			ImGui::Text("This cannot be undone. Are you sure?");
			if (ImGui::Button("Ok"))
			{
				m_FileDispatcher->EnqueueEvent(FileEvent{ .eAction = FileAction::Delete, .sFilepath = m_FilepathToAction });
				m_FilepathToAction.clear();
				m_FileAction = FileAction::NoAction;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				m_FileAction = FileAction::NoAction;

			ImGui::EndPopup();
		}
	}

}
