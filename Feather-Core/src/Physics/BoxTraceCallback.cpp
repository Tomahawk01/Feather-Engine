#include "BoxTraceCallback.h"

namespace Feather {

	bool BoxTraceCallback::ReportFixture(b2Fixture* fixture)
	{
		m_Bodies.push_back(fixture->GetBody());

		return true;
	}

}
