#pragma once

#include <fstream>
#include <sstream>
#include <concepts>

namespace Feather {

	template <class T>
	concept Streamable = requires(std::ostream & os, T obj) { os << obj; };

	template <Streamable T>
	std::string to_string(T val)
	{
		if constexpr (std::is_same_v<T, std::string>)
		{
			return val;
		}
		else
		{
			std::stringstream ss;
			ss << val;
			return ss.str();
		}
	}

	class LuaSerializer
	{
	public:
		explicit LuaSerializer(const std::string& filepath);
		~LuaSerializer();

		bool ResetStream(const std::string& newFilename);
		bool FinishStream();

		inline const std::string& GetFilepath() const { return m_Filepath; }

		LuaSerializer& AddComment(const std::string& comment);
		LuaSerializer& AddBlockComment(const std::string& comment);

		LuaSerializer& StartNewTable(const std::string& tableName = "", bool newLine = true, bool bracketed = false, bool quoted = false);
		LuaSerializer& EndTable(bool newLine = true);

		template <Streamable TValue>
		LuaSerializer& AddValue(const TValue& value, bool newLine = true, bool finalValue = false, bool indent = false, bool quote = false);

		template <Streamable TKey, Streamable TValue>
		LuaSerializer& AddKeyValuePair(TKey key, const TValue& value, bool addNewLine = true, bool finalValue = false, bool quoteKey = false, bool quoteValue = false);

		template<Streamable T>
		LuaSerializer& AddWords(T words, bool newLine = false, bool indent = false, int indentSize = 1);

	private:
		void AddIndents();
		void AddNewLine();
		void SeparateValues(bool newLine = true);

		std::string AddQuotes(const std::string& str);

		template <Streamable T>
		void Stream(const T& val);

	private:
		std::fstream m_FileStream;
		std::string m_Filepath;
		int m_NumIndents;
		int m_NumTablesStarted;
		bool m_StreamStarted;
		bool m_ValueAdded;
		bool m_NewLineAdded;
	};

}

#include "LuaSerializer.inl"
