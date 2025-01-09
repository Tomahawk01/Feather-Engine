#pragma once
#include "Gizmo.h"

namespace Feather {

	class TranslateGizmo : public Gizmo
	{
	public:
		TranslateGizmo();

		virtual void Update(struct Canvas& canvas) override;
		virtual void Draw() override;
	};

}
