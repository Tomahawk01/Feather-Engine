#include "IconReplacer.h"

#include "Logger/Logger.h"

#include <fstream>
#include <vector>

#ifdef _WIN32
#include <Windows.h>

#pragma pack(push, 2)
struct ICONDIR
{
	WORD reserved;
	WORD type;
	WORD count;
};

struct ICONDIRENTRY
{
	BYTE width;
	BYTE height;
	BYTE colorCount;
	BYTE reserved;
	BYTE planes;
	WORD bitCount;
	DWORD bytesInRes;
	DWORD imageOffset;
};

struct GRPICONDIR
{
	WORD reserved;
	WORD type;
	WORD count;
};

struct GRPICONDIRENTRY
{
	BYTE width;
	BYTE height;
	BYTE colorCount;
	BYTE reserved;
	WORD planes;
	WORD bitCount;
	DWORD bytesInRes;
	WORD id;
};
#pragma pack(pop)
#else
#endif

namespace Feather {

	class IconReplacer::Impl
	{
	public:
		Impl(const std::string& iconFile, const std::string& exeFile)
			: m_IconFile{ iconFile }
			, m_ExeFile{ exeFile }
		{}

#ifdef _WIN32
		bool ReplaceIcon_Win32();
#else
		bool ReplaceIcon_Linux();
#endif

	private:
		std::string m_IconFile;
		std::string m_ExeFile;
	};

	IconReplacer::IconReplacer(const std::string& iconFile, const std::string& exeFile)
		: m_Impl{ std::make_unique<Impl>(iconFile, exeFile) }
	{}

	IconReplacer::~IconReplacer() = default;

	bool IconReplacer::ReplaceIcon()
	{
#ifdef _WIN32
		return m_Impl->ReplaceIcon_Win32();
#else
		return m_Impl->ReplaceIcon_Linux();
#endif
	}

#ifdef _WIN32
	bool IconReplacer::Impl::ReplaceIcon_Win32()
	{
		std::ifstream file(m_IconFile, std::ios::binary);
		if (!file)
		{
			F_ERROR("Failed to replace icon: Icon file could not be opened");
			if (file.fail())
				F_ERROR("Failed to replace icon: Stream failbit set (invalid format or operation)");
			if (file.bad())
				F_ERROR("Failed to replace icon: Stream badbit set (irrecoverable error, e.g. hardware I/O failure)");
			if (!file.good())
				F_ERROR("Failed to replace icon: General stream if not good");

			return false;
		}

		ICONDIR iconDir;
		file.read(reinterpret_cast<char*>(&iconDir), sizeof(ICONDIR));
		if (iconDir.count != 1)
		{
			F_ERROR("Failed to replace icon: Image count is not 1 for {}\n"
					"Icons with multiple images are not supported",
					m_IconFile);
			return false;
		}

		ICONDIRENTRY entry;
		file.read(reinterpret_cast<char*>(&entry), sizeof(ICONDIRENTRY));

		std::vector<char> image(entry.bytesInRes);
		file.seekg(entry.imageOffset, std::ios::beg);
		file.read(image.data(), entry.bytesInRes);

		GRPICONDIR grpDir{ 0, 1, 1 };
		GRPICONDIRENTRY grpEntry{ entry.width, entry.height, entry.colorCount, entry.reserved, entry.planes, entry.bitCount, entry.bytesInRes, 1 };

		HANDLE update = BeginUpdateResourceA(m_ExeFile.c_str(), FALSE);
		if (!update)
		{
			F_ERROR("Failed to replace icon: Unable to open update handle");
			return false;
		}

		if (!UpdateResourceA(update, (LPCSTR)RT_ICON, MAKEINTRESOURCEA(1), 0, image.data(), entry.bytesInRes))
		{
			EndUpdateResourceA(update, TRUE);
			F_ERROR("Failed to update RT_ICON");
			return false;
		}

		std::vector<char> groupResource(sizeof(GRPICONDIR) + sizeof(GRPICONDIRENTRY));
		memcpy(groupResource.data(), &grpDir, sizeof(GRPICONDIR));
		memcpy(groupResource.data() + sizeof(GRPICONDIR), &grpEntry, sizeof(GRPICONDIRENTRY));

		if (!UpdateResourceA(update, (LPCSTR)RT_GROUP_ICON, MAKEINTRESOURCEA(1), 0, groupResource.data(), groupResource.size()))
		{
			EndUpdateResourceA(update, TRUE);
			F_ERROR("Failed to update RT_GROUP_ICON");
			return false;
		}

		return EndUpdateResourceA(update, FALSE);
	}
#else
	bool IconReplacer::Impl::ReplaceIcon_Linux()
	{
		return false;
	}
#endif

}
