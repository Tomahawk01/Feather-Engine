#include "RayCastCallback.h"

namespace Feather {

	RayCastCallback::RayCastCallback()
		: m_HitFixture{ nullptr }
		, m_Point{}
		, m_Normal{}
		, m_Fraction{}
		, m_Hit{ false }
	{}

	float RayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
	{
		m_Hit = true;
		m_HitFixture = fixture;
		m_Point = point;
		m_Normal = normal;
		m_Fraction = fraction;

		return 0.0f;
	}

}
