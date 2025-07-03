#include "ContentDisplay.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Core/Events/EventDispatcher.h"
#include "Core/CoreUtils/SaveProject.h"
#include "Utils/FeatherUtilities.h"
#include "Utils/HelperUtilities.h"
#include "FileSystem/Dialogs/FileDialog.h"
#include "FileSystem/Process/FileProcessor.h"
#include "FileSystem/Serializers/LuaSerializer.h"

#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Utilities/fonts/IconsFontAwesome5.h"
#include "Editor/Events/EditorEventTypes.h"

#include <imgui.h>

namespace Feather {

	ContentDisplay::ContentDisplay()
		: m_FileDispatcher{ std::make_unique<EventDispatcher>() }
		, m_CurrentDir{ MAIN_REGISTRY().GetContext<std::shared_ptr<SaveProject>>()->projectPath + "content" }
		, m_FilepathToAction{}
		, m_Selected{ -1 }
		, m_FileAction{ FileAction::NoAction }
		, m_CreateAction{ ContentCreateAction::NoAction }
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
			// F_ERROR("NumCols is zero!");
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

					std::string contentBtn = "##content_" + std::to_string(id);
					if (itr->is_directory())
					{
						// Change to the next Directory
						ImGui::ImageButton(contentBtn.c_str(), (ImTextureID)(intptr_t)icon->GetID(), ImVec2{ 80.0f, 80.0f });
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
						ImGui::ImageButton(contentBtn.c_str(), (ImTextureID)(intptr_t)icon->GetID(), ImVec2{ 80.0f, 80.0f });
						ImGui::PopStyleVar(1);

						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
						{
							FileProcessor fp{};
							if (!fp.OpenApplicationFromFile(path.string(), {}))
							{
								F_ERROR("Failed to open file {}", path.string());
							}
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
							ImGui::Selectable(ICON_FA_CUT "Cut");
							ImGui::EndDisabled();
						}
						else
						{
							if (ImGui::Selectable(ICON_FA_CUT "Cut"))
							{
								m_FilepathToAction = path.string();
								m_ItemCut = true;
							}
							if (ImGui::Selectable(ICON_FA_TRASH "Delete"))
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
						if (ImGui::Selectable(ICON_FA_PASTE "Paste"))
						{
							m_FileDispatcher->EnqueueEvent(FileEvent{ .eAction = FileAction::Paste });
						}
					}
					else
					{
						ImGui::BeginDisabled();
						ImGui::Selectable(ICON_FA_PASTE "Paste");
						ImGui::EndDisabled();
					}

					if (ImGui::Selectable(ICON_FA_FOLDER " New Folder"))
					{
						m_FilepathToAction = m_CurrentDir.string();
						m_CreateAction = ContentCreateAction::Folder;
					}

					if (ImGui::TreeNode("Lua Objects"))
					{
						if (ImGui::Selectable(ICON_FA_FILE " Create Lua Class"))
						{
							m_FilepathToAction = m_CurrentDir.string();
							m_CreateAction = ContentCreateAction::LuaClass;
						}
						ImGui::ItemToolTip("Generates an empty lua class.");

						if (ImGui::Selectable(ICON_FA_FILE " Create Lua Table"))
						{
							m_FilepathToAction = m_CurrentDir.string();
							m_CreateAction = ContentCreateAction::LuaTable;
						}
						ImGui::ItemToolTip("Generates an empty lua table");

						if (ImGui::Selectable(ICON_FA_FILE " Create Lua File"))
						{
							m_FilepathToAction = m_CurrentDir.string();
							m_CreateAction = ContentCreateAction::EmptyLuaFile;
						}
						ImGui::ItemToolTip("Generates an empty lua File.");

						ImGui::TreePop();
					}

					ImGui::EndPopup();
				}
			}

			ImGui::EndTable();
		}

		HandlePopups();

		ImGui::End();
	}

	void ContentDisplay::DrawToolbar()
	{
		ImGui::Separator();

		if (ImGui::Button(ICON_FA_FOLDER_PLUS))
		{
			m_FilepathToAction = m_CurrentDir.string();
			m_CreateAction = ContentCreateAction::Folder;
		}
		ImGui::ItemToolTip("Create Folder");
		ImGui::SameLine(0.0f, 16.0f);

		const auto& savedPath = MAIN_REGISTRY().GetContext<std::shared_ptr<SaveProject>>()->projectPath;
		std::string pathStr{ m_CurrentDir.string() };
		std::string pathToSplit = pathStr.substr(pathStr.find(savedPath) + savedPath.size());
		auto dir = SplitStr(pathToSplit, PATH_SEPARATOR);
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
			ImGui::Button(std::format("{}##{}", ICON_FA_ANGLE_RIGHT, i).c_str(), {16.0f, 18.0f});
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
				{
					std::unordered_set<std::string> filesToCheck;
					// Collect all regular file paths inside the directory before deleting it
					for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
					{
						if (entry.is_regular_file())
						{
							filesToCheck.insert(entry.path().string());
						}
					}

					// Recursively delete the directory and its contents
					std::filesystem::remove_all(path);

					// Remove each file from the asset manager
					for (const auto& filepath : filesToCheck)
					{
						if (!ASSET_MANAGER().DeleteAssetFromPath(filepath))
						{
							F_ERROR("Failed to remove asset from asset manager. '{}'", filepath);
						}
					}
				}
				else if (std::filesystem::is_regular_file(path))
				{
					// Remove the file from disk
					if (std::filesystem::remove(path))
					{
						// Remove the file from the asset manager
						if (!ASSET_MANAGER().DeleteAssetFromPath(path.string()))
						{
							F_ERROR("Failed to remove asset from asset manager. '{}'", path.string());
						}
					}
				}
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

	void ContentDisplay::HandleCreateEvent(const ContentCreateEvent& createEvent)
	{
		if (createEvent.eAction == ContentCreateAction::NoAction)
			return;

		switch (createEvent.eAction)
		{
			case ContentCreateAction::Folder: OpenCreateFolderPopup(); break;
		}
	}

	void ContentDisplay::HandlePopups()
	{
		if (m_FileAction != FileAction::NoAction)
		{
			switch (m_FileAction)
			{
				case FileAction::Delete: OpenDeletePopup(); break;
			}
		}

		if (m_CreateAction != ContentCreateAction::NoAction)
		{
			switch (m_CreateAction)
			{
				case ContentCreateAction::Folder: OpenCreateFolderPopup(); break;
				case ContentCreateAction::LuaClass: OpenCreateLuaClassPopup(); break;
				case ContentCreateAction::LuaTable: OpenCreateLuaTablePopup(); break;
				case ContentCreateAction::EmptyLuaFile: OpenCreateEmptyLuaFilePopup(); break;
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

	void ContentDisplay::OpenCreateFolderPopup()
	{
		if (m_CreateAction != ContentCreateAction::Folder)
			return;

		ImGui::OpenPopup("Create Folder");

		if (ImGui::BeginPopupModal("Create Folder"))
		{
			static std::string newFolderStr{};
			char temp[256];
			memset(temp, 0, sizeof(temp));
			strcpy_s(temp, newFolderStr.c_str());
			bool bNameEntered{ false }, bExit{ false };

			ImGui::Text("folder name");
			ImGui::SameLine();

			if (!ImGui::IsAnyItemActive())
				ImGui::SetKeyboardFocusHere();

			if (ImGui::InputText("##folder name", temp, sizeof(temp), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				newFolderStr = std::string{ temp };
				bNameEntered = true;
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				bExit = true;
			}

			static std::string errorText{};
			if (bNameEntered && !newFolderStr.empty())
			{
				std::string folderPathStr = m_CurrentDir.string() + PATH_SEPARATOR + newFolderStr;
				std::error_code error{};
				if (!std::filesystem::create_directory(std::filesystem::path{ folderPathStr }, error))
				{
					F_ERROR("Failed to create new folder: {}", error.message());
					errorText = "Failed to create new folder: " + error.message();
				}
				else
				{
					m_CreateAction = ContentCreateAction::NoAction;
					m_FilepathToAction.clear();
					newFolderStr.clear();
					errorText.clear();
					ImGui::CloseCurrentPopup();
				}
			}

			if (ImGui::Button("Cancel") || bExit)
			{
				m_CreateAction = ContentCreateAction::NoAction;
				m_FilepathToAction.clear();
				errorText.clear();
				newFolderStr.clear();
				ImGui::CloseCurrentPopup();
			}

			if (!errorText.empty())
			{
				ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, errorText.c_str());
			}

			ImGui::EndPopup();
		}
	}

	void ContentDisplay::OpenCreateLuaClassPopup()
	{
		if (m_CreateAction != ContentCreateAction::LuaClass)
			return;

		ImGui::OpenPopup("Create Lua Class");

		if (ImGui::BeginPopupModal("Create Lua Class"))
		{
			char buffer[256];
			static std::string className{};
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, className.c_str());
			bool bNameEntered{ false }, bExit{ false };
			ImGui::Text("Class Name");
			ImGui::SameLine();

			if (!ImGui::IsAnyItemActive())
				ImGui::SetKeyboardFocusHere();

			if (ImGui::InputText("##ClassName", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				className = std::string{ buffer };
				bNameEntered = true;
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				bExit = true;
			}

			static std::string errorText{};

			if (bNameEntered && !className.empty())
			{
				std::string filename = m_FilepathToAction + PATH_SEPARATOR + className + ".lua";

				if (std::filesystem::exists(std::filesystem::path{ filename }))
				{
					F_ERROR("Class file: '{}' already exists at '{}'", className, filename);
					errorText = "Class file: '" + className + "' already exists at '" + filename + "'";
				}
				else
				{
					LuaSerializer lw{ filename };

					lw.AddWords(className + " = {}")
						.AddWords(className + ".__index = " + className, true)
						.AddWords("function " + className + ":Create(params)", true)
						.AddWords("local this = {}", true, true)
						.AddWords("setmetatable(this, self)", true, true)
						.AddWords("return this", true, true)
						.AddWords("end", true);

					errorText.clear();
					className.clear();
					m_CreateAction = ContentCreateAction::NoAction;
					m_FilepathToAction.clear();
					ImGui::CloseCurrentPopup();
				}
			}

			if (ImGui::Button("Cancel") || bExit)
			{
				ImGui::CloseCurrentPopup();
				m_CreateAction = ContentCreateAction::NoAction;
				className.clear();
				errorText.clear();
				m_FilepathToAction.clear();
			}

			if (!errorText.empty())
			{
				ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, errorText.c_str());
			}

			ImGui::EndPopup();
		}
	}

	void ContentDisplay::OpenCreateLuaTablePopup()
	{
		if (m_CreateAction != ContentCreateAction::LuaTable)
			return;

		ImGui::OpenPopup("Create Lua Table");

		if (ImGui::BeginPopupModal("Create Lua Table"))
		{
			char buffer[256];
			static std::string tableName{};
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, tableName.c_str());
			bool bNameEntered{ false }, bExit{ false };
			ImGui::Text("Table Name");
			ImGui::SameLine();

			if (!ImGui::IsAnyItemActive())
				ImGui::SetKeyboardFocusHere();

			if (ImGui::InputText("##TableName", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				tableName = std::string{ buffer };
				bNameEntered = true;
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				bExit = true;
			}

			static std::string errorText{};

			if (bNameEntered && !tableName.empty())
			{
				std::string filename = m_FilepathToAction + PATH_SEPARATOR + tableName + ".lua";

				if (std::filesystem::exists(std::filesystem::path{ filename }))
				{
					F_ERROR("Table file: '{}' already exists at '{}'", tableName, filename);
					errorText = "Table file: '" + tableName + "' already exists at '" + filename + "'";
				}
				else
				{
					LuaSerializer lw{ filename };

					lw.StartNewTable(tableName).EndTable().FinishStream();

					errorText.clear();
					tableName.clear();
					m_CreateAction = ContentCreateAction::NoAction;
					m_FilepathToAction.clear();
					ImGui::CloseCurrentPopup();
				}
			}

			if (ImGui::Button("Cancel") || bExit)
			{
				ImGui::CloseCurrentPopup();
				m_CreateAction = ContentCreateAction::NoAction;
				tableName.clear();
				errorText.clear();
				m_FilepathToAction.clear();
			}

			if (!errorText.empty())
			{
				ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, errorText.c_str());
			}

			ImGui::EndPopup();
		}
	}

	void ContentDisplay::OpenCreateEmptyLuaFilePopup()
	{
		if (m_CreateAction != ContentCreateAction::EmptyLuaFile)
			return;

		ImGui::OpenPopup("Create Lua File");

		if (ImGui::BeginPopupModal("Create Lua File"))
		{
			char buffer[256];
			static std::string tableName{};
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, tableName.c_str());
			bool bNameEntered{ false }, bExit{ false };
			ImGui::Text("FileName");
			ImGui::SameLine();

			if (!ImGui::IsAnyItemActive())
				ImGui::SetKeyboardFocusHere();

			if (ImGui::InputText("##FileName", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				tableName = std::string{ buffer };
				bNameEntered = true;
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				bExit = true;
			}

			static std::string errorText{};

			if (bNameEntered && !tableName.empty())
			{
				std::string filename = m_FilepathToAction + PATH_SEPARATOR + tableName + ".lua";

				if (std::filesystem::exists(std::filesystem::path{ filename }))
				{
					F_ERROR("File '{}' already exists at '{}'", tableName, filename);
					errorText = "File '" + tableName + "' already exists at '" + filename + "'";
				}
				else
				{
					LuaSerializer lw{ filename };
					lw.FinishStream();

					errorText.clear();
					tableName.clear();
					m_CreateAction = ContentCreateAction::NoAction;
					m_FilepathToAction.clear();
					ImGui::CloseCurrentPopup();
				}
			}

			if (ImGui::Button("Cancel") || bExit)
			{
				ImGui::CloseCurrentPopup();
				m_CreateAction = ContentCreateAction::NoAction;
				tableName.clear();
				errorText.clear();
				m_FilepathToAction.clear();
			}

			if (!errorText.empty())
			{
				ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, errorText.c_str());
			}

			ImGui::EndPopup();
		}
	}

}
