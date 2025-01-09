#pragma once
#include "Gizmo.h"

namespace Feather {

	class ScaleGizmo : public Gizmo
	{
	public:
		ScaleGizmo();

		virtual void Update(struct Canvas& canvas) override;
		virtual void Draw() override;
	};

}
