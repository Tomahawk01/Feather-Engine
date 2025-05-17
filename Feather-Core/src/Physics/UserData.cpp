#include "UserData.h"

namespace Feather {

	bool ObjectData::AddContact(const ObjectData* objectData)
	{
		if (tag.empty() && group.empty())
			return false;

		if (objectData->tag.empty() && objectData->group.empty())
			return false;

		if (objectData->tag == tag && objectData->group == group)
			return false;

		if (isFriendly && objectData->isFriendly && isTrigger && objectData->isTrigger)
			return false;

		auto contactItr = std::find_if(
			contactEntities.begin(), contactEntities.end(),
			[&](const ObjectData* contactInfo) {
				return *contactInfo == *objectData;
			}
		);

		if (contactItr != contactEntities.end())
			return false;

		contactEntities.push_back(objectData);

		return true;
	}

	bool ObjectData::RemoveContact(const ObjectData* objectData)
	{
		if (objectData->tag.empty() && objectData->group.empty())
			return true;

		auto contactItr = std::remove_if(
			contactEntities.begin(), contactEntities.end(),
			[&](const ObjectData* contactInfo) {
				return *contactInfo == *objectData;
			}
		);

		if (contactItr == contactEntities.end())
			return false;

		contactEntities.erase(contactItr, contactEntities.end());

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
