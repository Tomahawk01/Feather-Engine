#include "Box2DWrappers.h"

namespace Feather {

	void BodyDestroyer::operator()(b2Body* body) const
	{
		body->GetWorld()->DestroyBody(body);
	}

}
