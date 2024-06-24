#pragma once
#include "IDisplay.h"

#include "imgui.h"

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
		bool m_AutoScroll;
	};

}