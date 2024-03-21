#pragma once

#include "Box2DWrappers.h"
#include "UserData.h"

namespace Feather {

	class ContactListener : public b2ContactListener
	{
	public:
		void BeginContact(b2Contact* contact) override;
		void EndContact(b2Contact* contact) override;

		void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;
		void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;

		UserData* GetUserDataA() { return m_UserDataA; }
		UserData* GetUserDataB() { return m_UserDataB; }

	private:
		UserData* m_UserDataA{ nullptr };
		UserData* m_UserDataB{ nullptr };

		void SetUserContacts(UserData* a, UserData* b);
	};

}
