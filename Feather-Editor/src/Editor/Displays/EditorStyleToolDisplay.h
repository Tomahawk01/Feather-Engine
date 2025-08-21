#pragma once

#include "IDisplay.h"

namespace Feather {

	class EditorStyleToolDisplay : public IDisplay
	{
	private:
		void DrawSizePicker();
		void DrawColorPicker();

	public:
		EditorStyleToolDisplay();
		virtual ~EditorStyleToolDisplay() = default;

		virtual void Draw() override;
	};

}
