#include "ContactListener.h"

#include "Logger/Logger.h"

#include <entt.hpp>

namespace Feather {

	void ContactListener::BeginContact(b2Contact* contact)
	{
		auto* fixtureA = contact->GetFixtureA();
		auto* fixtureB = contact->GetFixtureB();

		if (!fixtureA || !fixtureB || !fixtureA->GetUserData().pointer || !fixtureB->GetUserData().pointer)
		{
			SetUserContacts(nullptr, nullptr);
			return;
		}

		UserData* a_data = reinterpret_cast<UserData*>(fixtureA->GetUserData().pointer);
		UserData* b_data = reinterpret_cast<UserData*>(fixtureB->GetUserData().pointer);

		constexpr auto expectedType = entt::type_hash<ObjectData>::value();

		if (!a_data || !b_data || a_data->type_id != expectedType || b_data->type_id != expectedType)
		{
			SetUserContacts(nullptr, nullptr);
			return;
		}

		try
		{
			auto* a_any = std::any_cast<ObjectData>(&a_data->userData);
			auto* b_any = std::any_cast<ObjectData>(&b_data->userData);

			a_any->AddContact(b_any);
			b_any->AddContact(a_any);

			SetUserContacts(a_data, b_data);
		}
		catch (const std::bad_any_cast& ex)
		{
			// Ignore this exception for now
			//F_ERROR("Failed to cast user contacts: {}", ex.what());
			SetUserContacts(nullptr, nullptr);
		}
	}

	void ContactListener::EndContact(b2Contact* contact)
	{
		auto* fixtureA = contact->GetFixtureA();
		auto* fixtureB = contact->GetFixtureB();

		if (!fixtureA || !fixtureB || !fixtureA->GetUserData().pointer || !fixtureB->GetUserData().pointer)
		{
			SetUserContacts(nullptr, nullptr);
			return;
		}

		UserData* a_data = reinterpret_cast<UserData*>(fixtureA->GetUserData().pointer);
		UserData* b_data = reinterpret_cast<UserData*>(fixtureB->GetUserData().pointer);

		constexpr auto expectedType = entt::type_hash<ObjectData>::value();

		if (!a_data || !b_data || a_data->type_id != expectedType || b_data->type_id != expectedType)
		{
			SetUserContacts(nullptr, nullptr);
			return;
		}

		try
		{
			auto* a_any = std::any_cast<ObjectData>(&a_data->userData);
			auto* b_any = std::any_cast<ObjectData>(&b_data->userData);
			if (!a_any && b_any)
			{
				b_any->ClearContacts();
				SetUserContacts(nullptr, nullptr);
				return;
			}

			if (a_any && !b_any)
			{
				a_any->ClearContacts();
				SetUserContacts(nullptr, nullptr);
				return;
			}

			a_any->RemoveContact(b_any);
			b_any->RemoveContact(a_any);
		}
		catch (const std::bad_any_cast& ex)
		{
			// Ignore this exception for now
			//F_ERROR("Failed to cast user contacts: {}", ex.what());
		}

		SetUserContacts(nullptr, nullptr);
	}

	void ContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		// TODO: Could make some sol::functions that could be setup to be used
	}

	void ContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
		// TODO: Could make some sol::functions that could be setup to be used
	}

	void ContactListener::SetUserContacts(UserData* a, UserData* b)
	{
		m_UserDataA = a;
		m_UserDataB = b;
	}

}
