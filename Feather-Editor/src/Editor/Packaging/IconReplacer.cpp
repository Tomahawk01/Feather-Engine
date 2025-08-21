#include "IconReplacer.h"

#include "Logger/Logger.h"

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
			return false;
		}

		ICONDIR iconDir;
		file.read(reinterpret_cast<char*>(&iconDir), sizeof(ICONDIR));

		if (iconDir.count == 0)
		{
			F_ERROR("Failed to replace icon: No images found in icon file");
			return false;
		}

		std::vector<ICONDIRENTRY> entries(iconDir.count);
		for (int i = 0; i < iconDir.count; ++i)
		{
			file.read(reinterpret_cast<char*>(&entries[i]), sizeof(ICONDIRENTRY));
		}

		GRPICONDIR grpDir{ 0, 1, iconDir.count };
		std::vector<char> groupResource(sizeof(GRPICONDIR) + sizeof(GRPICONDIRENTRY) * iconDir.count);
		memcpy(groupResource.data(), &grpDir, sizeof(GRPICONDIR));

		HANDLE update = BeginUpdateResourceA(m_ExeFile.c_str(), FALSE);
		if (!update)
		{
			F_ERROR("Failed to replace icon: Unable to open update handle");
			return false;
		}

		for (int i = 0; i < iconDir.count; ++i)
		{
			std::vector<char> image(entries[i].bytesInRes);
			file.seekg(entries[i].imageOffset, std::ios::beg);
			file.read(image.data(), entries[i].bytesInRes);

			if (!UpdateResourceA(update, (LPCSTR)RT_ICON, MAKEINTRESOURCEA(i + 1), 0, image.data(), entries[i].bytesInRes))
			{
				EndUpdateResourceA(update, TRUE);
				F_ERROR("Failed to update RT_ICON for image {}", i + 1);
				return false;
			}

			GRPICONDIRENTRY grpEntry{
				entries[i].width,
				entries[i].height,
				entries[i].colorCount,
				entries[i].reserved,
				entries[i].planes,
				entries[i].bitCount,
				entries[i].bytesInRes,
				static_cast<WORD>(i + 1)
			};

			memcpy(groupResource.data() + sizeof(GRPICONDIR) + sizeof(GRPICONDIRENTRY) * i, &grpEntry, sizeof(GRPICONDIRENTRY));
		}

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
