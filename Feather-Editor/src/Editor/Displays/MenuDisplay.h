#pragma once

#include "IDisplay.h"

namespace Feather {

	class EditorState;
	enum class EDisplay : uint64_t;

	class MenuDisplay : public IDisplay
	{
	public:
		MenuDisplay() = default;
		virtual ~MenuDisplay() = default;

		virtual void Draw() override;

	private:
		void DrawDisplayItem(EditorState& editorState, const std::string& displayName, const EDisplay display);
	};

}
