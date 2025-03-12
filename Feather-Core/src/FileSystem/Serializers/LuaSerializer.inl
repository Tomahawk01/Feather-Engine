#include "LuaSerializer.h"

namespace Feather {

	template<Streamable TValue>
	inline LuaSerializer& LuaSerializer::AddValue(const TValue& value, bool newLine, bool finalValue, bool indent)
	{
		SeparateValues(newLine);
		Stream(value);

		if (indent)
			++m_NumIndents;

		m_NewLineAdded = false;
		m_ValueAdded = !finalValue;

		return *this;
	}

	template<Streamable TKey, Streamable TValue>
	inline LuaSerializer& LuaSerializer::AddKeyValuePair(TKey key, const TValue& value, bool addNewLine, bool finalValue, bool quoteKey, bool quoteValue)
	{
		SeparateValues(addNewLine);

		if (quoteKey)
		{
			Stream("[" + AddQuotes(to_string(key)));
			Stream("] = ");
		}
		else
		{
			Stream(key);
			Stream(" = ");
		}

		if (quoteValue)
		{
			Stream(AddQuotes(to_string(value)));
		}
		else
		{
			Stream(value);
		}

		m_NewLineAdded = false;
		m_ValueAdded = !finalValue;

		return *this;
	}

	template<Streamable T>
	inline LuaSerializer& LuaSerializer::AddWords(T words, bool newLine, bool indent, int indentSize)
	{
		if (newLine)
			AddNewLine();

		if (indent)
		{
			m_NumIndents += indentSize;
			AddIndents();
			m_NumIndents -= indentSize;
		}

		Stream(words);
		m_NewLineAdded = false;
		m_ValueAdded = false;

		return *this;
	}

	template<Streamable T>
	inline void LuaSerializer::Stream(const T& val)
	{
		m_FileStream << val;
	}

}
