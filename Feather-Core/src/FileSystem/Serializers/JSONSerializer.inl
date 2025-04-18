#include "JSONSerializer.h"

namespace Feather {

	template<typename TValue>
	inline JSONSerializer& JSONSerializer::AddKeyValuePair(const std::string& key, const TValue& value)
	{
		m_Writer->Key(key.c_str());

		if constexpr (std::is_same_v<TValue, std::string>)
			m_Writer->String(value.c_str());
		else if constexpr (std::is_same_v<TValue, const char*>)
			m_Writer->String(value);
		else if constexpr (std::is_same_v<TValue, const char>)
			m_Writer->String(value);
		else if constexpr (std::is_integral_v<TValue>)
			m_Writer->Int64(value);
		else if constexpr (std::is_unsigned_v<TValue>)
			m_Writer->Uint64(value);
		else if constexpr (std::is_floating_point_v<TValue>)
			m_Writer->Double(value);
		else
			assert(false && "Type not supported!");

		return *this;
	}

}
