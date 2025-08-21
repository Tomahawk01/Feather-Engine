#include "LuaSerializer.h"

#include "Logger/Logger.h"

constexpr char SEPARATOR = ',';
constexpr char INDENT = '\t';
constexpr char NEW_LINE = '\n';
constexpr char SPACE = ' ';

#define SPECIAL_CASES					\
	std::set<char>						\
	{									\
		'"', '\\', '\n', '\t', '\r'		\
	}

namespace Feather {

	LuaSerializer::LuaSerializer(const std::string& filepath)
		: m_FileStream{}
		, m_Filepath{ filepath }
		, m_NumIndents{ 0 }
		, m_NumTablesStarted{ 0 }
		, m_StreamStarted{ false }
		, m_ValueAdded{ false }
		, m_NewLineAdded{ false }
	{
		m_FileStream.open(filepath, std::ios::out | std::ios::trunc);
		F_ASSERT(m_FileStream.is_open() /*&&
					  std::format( "LuaSerialization failed. Failed to open file '{}'", filename )*/);

		if (!m_FileStream.is_open())
			throw std::runtime_error(std::format("LuaSerialization failed. Failed to open file '{}'", filepath));
	}

	LuaSerializer::~LuaSerializer()
	{
		if (m_FileStream.is_open())
			m_FileStream.close();
	}

	bool LuaSerializer::ResetStream(const std::string& newFilename)
	{
		if (m_FileStream.is_open())
		{
			m_FileStream.close();
			m_NewLineAdded = false;
			m_ValueAdded = false;
			m_StreamStarted = false;
			m_Filepath = newFilename;
		}

		m_FileStream.open(newFilename, std::ios::out | std::ios::trunc);
		return m_FileStream.is_open();
	}

	bool LuaSerializer::FinishStream()
	{
		F_ASSERT(m_NumTablesStarted == 0 && "Too many tables started! Did you forget to call EndTable()?");
		F_ASSERT(m_NumIndents == 0 && "Indent count should be zero when ending the document!");
		Stream(NEW_LINE);
		return true;
	}

	LuaSerializer& LuaSerializer::AddComment(const std::string& comment)
	{
		Stream("-- " + comment + "\n");
		return *this;
	}

	LuaSerializer& LuaSerializer::AddBlockComment(const std::string& comment)
	{
		Stream("--[[\n" + comment);
		Stream('\n');
		Stream("--]]\n");
		return *this;
	}

	LuaSerializer& LuaSerializer::StartNewTable(const std::string& tableName, bool newLine, bool bracketed, bool quoted)
	{
		++m_NumTablesStarted;
		SeparateValues(newLine);

		if (!tableName.empty())
		{
			if (bracketed)
			{
				Stream("[" + (quoted ? AddQuotes(tableName) : tableName) + "]");
			}
			else
			{
				Stream(tableName);
			}

			Stream(" = { ");
		}
		else // Table with no key --> should use integers
		{
			Stream("{ ");
		}

		++m_NumIndents;
		m_NewLineAdded = false;
		m_ValueAdded = false;

		return *this;
	}

	LuaSerializer& LuaSerializer::EndTable(bool newLine)
	{
		--m_NumTablesStarted;
		--m_NumIndents;
		SeparateValues(newLine);
		Stream('}');
		m_NewLineAdded = false;
		m_ValueAdded = true;

		return *this;
	}

	void LuaSerializer::AddIndents()
	{
		std::string indent{};
		for (int i = 0; i < m_NumIndents; i++)
		{
			indent += INDENT;
		}

		Stream(indent);
	}

	void LuaSerializer::AddNewLine()
	{
		if (m_NewLineAdded)
			return;

		Stream(NEW_LINE);
		AddIndents();
	}

	void LuaSerializer::SeparateValues(bool newLine)
	{
		if (!m_ValueAdded && newLine)
		{
			AddNewLine();
			return;
		}

		if (m_ValueAdded)
		{
			Stream(SEPARATOR);
			Stream(SPACE);
		}

		m_ValueAdded = false;
		if (newLine)
			AddNewLine();
	}

	std::string LuaSerializer::AddQuotes(const std::string& str)
	{
		std::string quotedStr{ "\"" };
		std::string stringToQuote{ str };

		auto it = stringToQuote.begin();
		while (it != stringToQuote.end())
		{
			it = std::find_if(it, stringToQuote.end(), [&](char c) { return SPECIAL_CASES.contains(c); });

			if (it != stringToQuote.end())
				it = stringToQuote.insert(it + 1, '\\') + 1;
		}

		quotedStr += stringToQuote + "\"";
		return quotedStr;
	}

}
