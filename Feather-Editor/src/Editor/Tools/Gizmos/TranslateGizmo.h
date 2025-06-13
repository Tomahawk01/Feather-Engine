#pragma once
#include "Gizmo.h"

namespace Feather {

	class TranslateGizmo : public Gizmo
	{
	public:
		TranslateGizmo();

		virtual void Update(Canvas& canvas) override;
		virtual void Draw(Camera2D* camera) override;
	};

}
