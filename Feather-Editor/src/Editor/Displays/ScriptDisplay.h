#pragma once

#include "IDisplay.h"

namespace Feather {

	class DirectoryWatcher;

	class ScriptDisplay : public IDisplay
	{
	public:
		ScriptDisplay();
		virtual ~ScriptDisplay();

		virtual void Draw() override;
		virtual void Update() override;

	private:
		void GenerateScriptList();
		void WriteScriptListToFile();
		void OnFileChanged(const std::filesystem::path& path, bool modified);

	private:
		std::vector<std::string> m_ScriptList;
		std::vector<std::string> m_Scripts;
		std::string m_ScriptsDirectory;
		int m_Selected;
		bool m_ScriptsChanged;
		bool m_ListScripts;

		std::unique_ptr<DirectoryWatcher> m_DirWatcher;
		std::atomic_bool m_FilesChanged;
	};

}
