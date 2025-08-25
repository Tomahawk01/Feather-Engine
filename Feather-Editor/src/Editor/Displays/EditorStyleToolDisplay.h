#pragma once

#include "IDisplay.h"

namespace Feather {

	class EditorStyleToolDisplay : public IDisplay
	{
	public:
		EditorStyleToolDisplay();
		virtual ~EditorStyleToolDisplay() = default;

		virtual void Draw() override;

	private:
		void DrawSizePicker();
		void DrawColorPicker();
	};

}
