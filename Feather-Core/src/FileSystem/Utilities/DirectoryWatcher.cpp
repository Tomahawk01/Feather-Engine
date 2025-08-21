#include "DirectoryWatcher.h"

#include "Logger/Logger.h"

#ifdef _WIN32
#include <Windows.h>
#else
// TODO: Add Linux headers?
#endif

namespace fs = std::filesystem;
using namespace std::chrono_literals;

namespace Feather {

	struct DirectoryWatcher::Impl
	{
		fs::path rootPath;
		Callback callback;
		std::atomic_bool stopFlag{ false };
		std::thread watcherThread;

#ifdef _WIN32
		HANDLE directoryHandle{ nullptr };
		HANDLE shutdownHandle{ nullptr };
		OVERLAPPED overlapped{};
#else
		// TODO: Add variables for Linux?
#endif

		Impl(const fs::path& path, Callback cb)
			: rootPath{ path }
			, callback{ std::move(cb) }
		{
			watcherThread = std::thread([this] { Run(); });
		}

		~Impl();

		void Run();

#ifdef _WIN32
		void RunWindows();
#else
		void RunLinux();
#endif
	};

	DirectoryWatcher::DirectoryWatcher(const fs::path& path, Callback callback)
		: m_Impl{ std::make_unique<Impl>(path, std::move(callback)) }
	{}

	DirectoryWatcher::~DirectoryWatcher() = default;

	DirectoryWatcher::Impl::~Impl()
	{
		stopFlag = true;
		if (watcherThread.joinable())
		{
#ifdef _WIN32
			SetEvent(shutdownHandle);
			CancelIoEx(directoryHandle, &overlapped);
#else
#endif
			watcherThread.join();
		}

#ifdef _WIN32
		if (directoryHandle && directoryHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(directoryHandle);
			directoryHandle = nullptr;
		}

		if (shutdownHandle && shutdownHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(shutdownHandle);
			shutdownHandle = nullptr;
		}
#else
		// TODO: Handle Linux shutdown stuff?
#endif
	}

	void DirectoryWatcher::Impl::Run()
	{
#ifdef _WIN32
		RunWindows();
#else
		RunLinux();
#endif
	}

#ifdef _WIN32
	void DirectoryWatcher::Impl::RunWindows()
	{
		constexpr DWORD bufferSize = 8192;
		BYTE buffer[bufferSize];

		HANDLE hDir = CreateFileW(
			rootPath.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			nullptr,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			nullptr);

		if (hDir == INVALID_HANDLE_VALUE)
		{
			F_ERROR("Failed to open directory: {}", GetLastError());
			return;
		}

		directoryHandle = hDir;

		shutdownHandle = CreateEventW(nullptr, TRUE, FALSE, nullptr);

		// NOTE: Event to receive async notifications from ReadDirectoryChangesW()
		HANDLE hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		overlapped.hEvent = hEvent;

		HANDLE handles[] = { hEvent, shutdownHandle };

		while (!stopFlag)
		{
			DWORD bytesReturned{ 0 };

			if (!hEvent)
			{
				F_ERROR("Failed to receive directory notifications");
				break;
			}

			ResetEvent(hEvent);

			BOOL result = ReadDirectoryChangesW(
				hDir,
				buffer, bufferSize,
				TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
				&bytesReturned, &overlapped, nullptr);

			if (!result && GetLastError() != ERROR_IO_PENDING)
			{
				F_ERROR("ReadDirectoryChangesW failed: {}", GetLastError());
				break;
			}

			// NOTE: Wait for either read to complete or the shutdown events
			DWORD waitStatus = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
			if (waitStatus == WAIT_OBJECT_0) // Overlapped event signaled
			{
				if (!GetOverlappedResult(hDir, &overlapped, &bytesReturned, FALSE))
				{
					DWORD err = GetLastError();
					if (err == ERROR_OPERATION_ABORTED)
						break;

					F_ERROR("GetOverlappedResult failed: {}", err);
					break;
				}

				FILE_NOTIFY_INFORMATION* notify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
				do
				{
					std::wstring filename{ notify->FileName, notify->FileNameLength / sizeof(WCHAR) };
					fs::path changedPath = rootPath / filename;

					if (callback)
					{
						bool modified = notify->Action == FILE_ACTION_MODIFIED ||
										notify->Action == FILE_ACTION_RENAMED_NEW_NAME ||
										notify->Action == FILE_ACTION_ADDED;

						callback(changedPath, modified);
					}

					if (notify->NextEntryOffset == 0)
						break;

					notify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<BYTE*>(notify) + notify->NextEntryOffset);
				} while (true);
			}
			else if (waitStatus == WAIT_OBJECT_0 + 1) // Shutdown event signaled
			{
				CancelIoEx(hDir, &overlapped);
				break;
			}
			else
			{
				F_ERROR("WaitForMultipleObjects failed: {}", GetLastError());
				break;
			}
		}

		if (hEvent)
			CloseHandle(hEvent);
		if (hDir)
			CloseHandle(hDir);

		directoryHandle = nullptr;
	}
#else
	void DirectoryWatcher::Impl::RunLinux()
	{
	}
#endif

}
