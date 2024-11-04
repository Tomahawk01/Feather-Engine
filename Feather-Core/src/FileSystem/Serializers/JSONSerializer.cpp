#include "JSONSerializer.h"
#include "Logger/Logger.h"

constexpr int MAX_DECIMAL_PLACES = 3;

namespace Feather {

	JSONSerializer::JSONSerializer(const std::string& filename)
		: m_FileStream{}, m_StringBuffer{},
		m_Writer{ std::make_unique<rapidjson::PrettyWriter<rapidjson::StringBuffer>>(m_StringBuffer) },
		m_NumObjectsStarted{ 0 }, m_NumArraysStarted{ 0 }
	{
		m_FileStream.open(filename, std::ios::out | std::ios::trunc);
		F_ASSERT(m_FileStream.is_open() && "Failed to open file!");

		if (!m_FileStream.is_open())
			throw std::runtime_error(std::format("JSONSerializer failed to open file '{}'", filename));

		m_Writer->SetMaxDecimalPlaces(MAX_DECIMAL_PLACES);
	}

	JSONSerializer::~JSONSerializer()
	{
		if (m_FileStream.is_open())
			m_FileStream.close();
	}

	bool JSONSerializer::StartDocument()
	{
		F_ASSERT(m_NumObjectsStarted == 0 && "Document has already been started. Please reset the serializer!");

		if (m_NumObjectsStarted != 0)
		{
			F_ERROR("Document has already been started. Please reset the serializer!");
			return false;
		}
		++m_NumObjectsStarted;

		return m_Writer->StartObject();
	}

	bool JSONSerializer::EndDocument()
	{
		F_ASSERT(m_NumObjectsStarted == 1 && "There should only be one object open");
		F_ASSERT(m_NumArraysStarted == 0 && "Too many arrays started. Did you forget to call EndArray()?");

		if (m_NumObjectsStarted != 1)
		{
			F_ERROR("Failed to end document correctly. There should only be one object open. Did you forget to call EndObject()?");
			return false;
		}
		if (m_NumArraysStarted != 0)
		{
			F_ERROR("Failed to end document correctly. Too many arrays started. Did you forget to call EndArray()?");
			return false;
		}

		m_Writer->EndObject();
		--m_NumObjectsStarted;

		m_FileStream << m_StringBuffer.GetString();
		m_FileStream.flush();

		return true;
	}

	bool JSONSerializer::Reset(const std::string& filename)
	{
		F_ASSERT(m_NumObjectsStarted == 0 && "Cannot reset, document has not been finished");
		F_ASSERT(m_NumArraysStarted == 0 && "Cannot reset, too many arrays open");

		if (m_NumObjectsStarted != 0)
		{
			F_ERROR("Cannot reset, document has not been finished");
			return false;
		}
		if (m_NumArraysStarted != 0)
		{
			F_ERROR("Cannot reset, too many arrays open");
			return false;
		}

		if (m_FileStream.is_open())
			m_FileStream.close();

		m_FileStream.open(filename, std::ios::out | std::ios::trunc);
		F_ASSERT(m_FileStream.is_open() && "Failed to open file!");

		if (!m_FileStream.is_open())
			throw std::runtime_error(std::format("JSONSerializer failed to open file '{}'", filename));

		return true;
	}

	JSONSerializer& JSONSerializer::StartNewObject(const std::string& key)
	{
		++m_NumObjectsStarted;
		if (!key.empty())
			m_Writer->Key(key.c_str());
		m_Writer->StartObject();
		return *this;
	}

	JSONSerializer& JSONSerializer::EndObject()
	{
		F_ASSERT(m_NumObjectsStarted > 1 && "EndObject() called too many times!");
		--m_NumObjectsStarted;
		m_Writer->EndObject();
		return *this;
	}

	JSONSerializer& JSONSerializer::StartNewArray(const std::string& key)
	{
		++m_NumArraysStarted;
		m_Writer->Key(key.c_str());
		m_Writer->StartArray();
		return *this;
	}

	JSONSerializer& JSONSerializer::EndArray()
	{
		F_ASSERT(m_NumArraysStarted > 0 && "EndArray() called too many times!");
		--m_NumArraysStarted;
		m_Writer->EndArray();
		return *this;
	}

	JSONSerializer& JSONSerializer::AddKeyValuePair(const std::string& key, const bool& value)
	{
		m_Writer->Key(key.c_str());
		m_Writer->Bool(value);

		return *this;
	}

}
