#include "FileProcessor.h"

#include "Utils/FeatherUtilities.h"
#include "Logger/Logger.h"

#include <Windows.h>
#include <shellapi.h>

namespace Feather {

	bool FileProcessor::OpenApplicationFromFile(const std::string& filename, std::vector<const char*> params)
	{
		std::string sParams = SeparateParams(params);

		INT_PTR shell = reinterpret_cast<INT_PTR>(
			::ShellExecuteW(
				NULL,
				L"open",
				ConvertAnsiToWide(filename).c_str(),
				!sParams.empty() ? ConvertAnsiToWide(sParams).c_str() : L"",
				L"",
				SW_SHOWNORMAL));

		// If there is no default program set for the file type, prompt the user to choose an application
		if (shell == SE_ERR_NOASSOC || shell == SE_ERR_ASSOCINCOMPLETE)
		{
			shell = reinterpret_cast<INT_PTR>(
				::ShellExecuteW(
					NULL,
					L"open",
					L"RUNDLL32.EXE",
					ConvertAnsiToWide(std::format("shell32.dll, OpenAs_RunDLL {}", filename)).c_str(),
					L"",
					SW_SHOWNORMAL));
		}

		// If the shell code us greater that 32 == SUCCESS
		if (shell >= 32)
			return true;

		DWORD error{ GetLastError() };
		F_ERROR("Failed to open file '{}': {}", filename, error);
		return true;
	}

}
