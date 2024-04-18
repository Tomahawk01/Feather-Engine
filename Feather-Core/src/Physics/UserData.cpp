#include "UserData.h"

namespace Feather {

	bool ObjectData::AddContact(const ObjectData& objectData)
	{
		auto contactItr = std::find_if(
			contactEntities.begin(), contactEntities.end(),
			[&](ObjectData& contactInfo) {
				return contactInfo == objectData;
			}
		);

		if (contactItr != contactEntities.end())
			return false;

		contactEntities.push_back(objectData);
		return true;
	}

	bool ObjectData::RemoveContact(const ObjectData& objectData)
	{
		auto contactItr = std::remove_if(
			contactEntities.begin(), contactEntities.end(),
			[&](ObjectData& contactInfo) {
				return contactInfo == objectData;
			}
		);

		if (contactItr == contactEntities.end())
			return false;

		contactEntities.erase(contactItr);
		return true;
	}

	std::string ObjectData::to_string() const
	{
		std::stringstream ss;
		ss << "==== Object Data ====\n" << std::boolalpha <<
			"Tag: " << tag << "\n" <<
			"Group: " << group << "\n" <<
			"isCollider: " << isCollider << "\n" <<
			"isTrigger: " << isTrigger << "\n" <<
			"EntityID: " << entityID << "\n";

		return ss.str();
	}

	bool operator==(const ObjectData& a, const ObjectData& b)
	{
		return a.isCollider == b.isCollider &&
			   a.isTrigger == b.isTrigger &&
			   a.tag == b.tag &&
			   a.group == b.group &&
			   a.entityID == b.entityID;
	}

}
