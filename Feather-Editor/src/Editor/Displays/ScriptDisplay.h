#pragma once

#include "IDisplay.h"

#include <vector>
#include <string>

namespace Feather {

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

	private:
		std::vector<std::string> m_ScriptList;
		std::vector<std::string> m_Scripts;
		std::string m_ScriptsDirectory;
		int m_Selected;
		bool m_ScriptsChanged;
	};

}
