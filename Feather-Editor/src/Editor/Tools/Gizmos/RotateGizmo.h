#pragma once
#include "Gizmo.h"

namespace Feather {

	class RotateGizmo : public Gizmo
	{
	public:
		RotateGizmo();

		virtual void Update(Canvas& canvas) override;
		virtual void Draw(Camera2D* camera) override;
	};

}
