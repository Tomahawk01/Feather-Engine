#pragma once

#include <entt.hpp>

namespace Feather {

	struct UserData
	{
		std::any userData{};
		std::uint32_t type_id{ 0 }; // For type hash from entt::meta
	};

	struct ObjectData
	{
		std::string tag{};
		std::string group{};
		bool isCollider{ false };
		bool isTrigger{ false };
		bool isFriendly{ false };
		std::uint32_t entityID{};
		
		ObjectData() = default;
		ObjectData(const std::string& tag, const std::string& group, bool collider, bool trigger, bool friendly, uint32_t entityId = entt::null);

		inline const std::vector<const ObjectData*>& GetContactEntities() const { return contactEntities; }

		friend bool operator==(const ObjectData& a, const ObjectData& b);
		[[nodiscard]] std::string to_string() const;

	private:
		bool AddContact(const ObjectData* objectData);
		bool RemoveContact(const ObjectData* objectData);
		void ClearContacts() { contactEntities.clear(); }

		friend class ContactListener;

	private:
		std::vector<const ObjectData*> contactEntities;
	};

}
