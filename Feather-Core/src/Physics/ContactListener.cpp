#include "ContactListener.h"

namespace Feather {

	void ContactListener::BeginContact(b2Contact* contact)
	{
		UserData* a_data = reinterpret_cast<UserData*>(contact->GetFixtureA()->GetUserData().pointer);
		UserData* b_data = reinterpret_cast<UserData*>(contact->GetFixtureB()->GetUserData().pointer);

		SetUserContacts(a_data, b_data);
	}

	void ContactListener::EndContact(b2Contact* contact)
	{
		SetUserContacts(nullptr, nullptr);
	}

	void ContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		// TODO:
	}

	void ContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
		// TODO:
	}

	void ContactListener::SetUserContacts(UserData* a, UserData* b)
	{
		m_UserDataA = a;
		m_UserDataB = b;
	}

}
