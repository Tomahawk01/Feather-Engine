#pragma once
#include "IDisplay.h"

namespace Feather {

	class MenuDisplay : public IDisplay
	{
	public:
		MenuDisplay() = default;
		virtual ~MenuDisplay() = default;

		virtual void Draw() override;

	private:
	};

}
