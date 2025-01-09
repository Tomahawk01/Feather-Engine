#pragma once
#include "Gizmo.h"

namespace Feather {

	class RotateGizmo : public Gizmo
	{
	public:
		RotateGizmo();

		virtual void Update(struct Canvas& canvas) override;
		virtual void Draw() override;
	};

}
