#pragma once
#include "Gizmo.h"

namespace Feather {

	class ScaleGizmo : public Gizmo
	{
	public:
		ScaleGizmo();

		virtual void Update(Canvas& canvas) override;
		virtual void Draw(Camera2D* camera) override;
	};

}
