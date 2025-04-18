#pragma once
#include <rapidjson/prettywriter.h>

#include <fstream>

namespace Feather {

	class JSONSerializer
	{
	public:
		JSONSerializer(const std::string& filename, int maxDecimalPlaces = -1);
		~JSONSerializer();

		bool StartDocument();
		bool EndDocument();

		bool Reset(const std::string& filename);

		JSONSerializer& StartNewObject(const std::string& key = "");
		JSONSerializer& EndObject();
		JSONSerializer& StartNewArray(const std::string& key);
		JSONSerializer& EndArray();

		JSONSerializer& AddKeyValuePair(const std::string& key, const bool& value);

		template <typename TValue>
		JSONSerializer& AddKeyValuePair(const std::string& key, const TValue& value);

	private:
		std::fstream m_FileStream;
		rapidjson::StringBuffer m_StringBuffer;
		std::unique_ptr<rapidjson::PrettyWriter<rapidjson::StringBuffer>> m_Writer;
		int m_NumObjectsStarted;
		int m_NumArraysStarted;
	};

}

#include "JSONSerializer.inl"
