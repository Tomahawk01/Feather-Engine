#pragma once

#include <box2d/box2d.h>

namespace Feather {

    class RayCastCallback : public b2RayCastCallback
    {
    public:
        RayCastCallback();

        virtual float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;

        inline const bool IsHit() const { return m_Hit; }
        inline b2Fixture* HitFixture() { return m_HitFixture; }
        inline const b2Vec2 HitPoint() const { return m_Point; }
        inline const b2Vec2 HitNormal() const { return m_Normal; }
        inline const float HitFraction() const { return m_Fraction; }

    private:
        b2Fixture* m_HitFixture;
        b2Vec2 m_Point;
        b2Vec2 m_Normal;
        float m_Fraction;
        bool m_Hit;
    };

}
