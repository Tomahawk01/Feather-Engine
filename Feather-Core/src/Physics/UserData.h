#pragma once

#include <any>
#include <string>
#include <sstream>

namespace Feather {

	struct UserData
	{
		std::any userData{};
		std::uint32_t type_id{ 0 }; // For type hash from entt::meta
	};

	struct ObjectData
	{
		std::string tag{ "" }, group{ "" };
		bool isCollider{ false }, isTrigger{ false };
		std::uint32_t entityID{};

		[[nodiscard]] std::string to_string() const
		{
			std::stringstream ss;
			ss <<
				"==== Object Data ====\n" << std::boolalpha <<
				"Tag: " << tag << "\n" <<
				"Group: " << group << "\n" <<
				"isCollider: " << isCollider << "\n" <<
				"isTrigger: " << isTrigger << "\n" <<
				"EntityID: " << entityID << "\n";
			
			return ss.str();
		}
	};

}
