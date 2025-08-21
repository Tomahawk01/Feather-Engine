#pragma once

#include "IDisplay.h"

#include <imgui.h>

namespace Feather {

	class LogDisplay : public IDisplay
	{
	public:
		LogDisplay();
		~LogDisplay() = default;

		void Clear();
		virtual void Draw() override;

	private:
		void GetLogs();

	private:
		ImGuiTextBuffer m_TextBuffer;
		ImVector<int> m_TextOffsets;
		std::string m_SearchQuery;
		bool m_AutoScroll;
		bool m_ShowTrace;
		bool m_ShowInfo;
		bool m_ShowWarn;
		bool m_ShowError;
		int m_LogIndex;
	};

}
