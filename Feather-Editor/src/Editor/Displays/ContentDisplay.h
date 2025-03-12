#pragma once

#include "IDisplay.h"

#include <string>
#include <filesystem>

namespace Feather {

	enum class FileAction;
	enum class EContentCreateAction;
	struct FileEvent;
	struct ContentCreateEvent;

	class EventDispatcher;

	class ContentDisplay : public IDisplay
	{
	public:
		ContentDisplay();
		virtual ~ContentDisplay();
		virtual void Update() override;
		virtual void Draw() override;

	private:
		virtual void DrawToolbar() override;
		void CopyDroppedFile(const std::string& fileToCopy, const std::filesystem::path& droppedPath);
		void MoveFolderOrFile(const std::filesystem::path& movedPath, const std::filesystem::path& path);
		void HandleFileEvent(const FileEvent& fileEvent);
		void HandleCreateEvent(const ContentCreateEvent& createEvent);
		void HandlePopups();

		void OpenDeletePopup();
		void OpenCreateFolderPopup();

		void OpenCreateLuaClassPopup();
		void OpenCreateLuaTablePopup();

	private:
		std::unique_ptr<EventDispatcher> m_FileDispatcher;
		std::filesystem::path m_CurrentDir;
		std::string m_FilepathToAction;
		int m_Selected;
		FileAction m_FileAction;
		EContentCreateAction m_CreateAction;

		bool m_ItemCut;
		bool m_WindowHovered;
	};

}
