#pragma once

#include "IDisplay.h"

#include <string>
#include <filesystem>

namespace Feather {

	enum class FileAction;
	struct FileEvent;

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
		void OpenDeletePopup();

	private:
		std::unique_ptr<EventDispatcher> m_FileDispatcher;
		std::filesystem::path m_CurrentDir;
		std::string m_FilepathToAction;
		int m_Selected;
		FileAction m_FileAction;
		bool m_ItemCut, m_WindowHovered;
	};

}
