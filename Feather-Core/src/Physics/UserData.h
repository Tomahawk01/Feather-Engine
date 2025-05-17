#pragma once

#include <any>
#include <string>
#include <sstream>
#include <vector>

namespace Feather {

	struct UserData
	{
		std::any userData{};
		std::uint32_t type_id{ 0 }; // For type hash from entt::meta
	};

	struct ObjectData
	{
		std::string tag{ "" };
		std::string group{ "" };
		bool isCollider{ false };
		bool isTrigger{ false };
		bool isFriendly{ false };
		std::uint32_t entityID{};
		std::vector<const ObjectData*> contactEntities;

		friend bool operator==(const ObjectData& a, const ObjectData& b);
		[[nodiscard]] std::string to_string() const;

	private:
		bool AddContact(const ObjectData* objectData);
		bool RemoveContact(const ObjectData* objectData);

		friend class ContactListener;
	};

}
