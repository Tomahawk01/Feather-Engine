#pragma once

#include <rapidjson/document.h>

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <shared_mutex>

namespace Feather {

	class ProjectInfo;
	class ThreadPool;
	struct GameConfig;

	struct PackageData
	{
		std::unique_ptr<ProjectInfo> ProjectInfo{ nullptr };
		std::unique_ptr<GameConfig> GameConfig{ nullptr };
		std::string TempDataPath{};
		std::string AssetFilepath{};
		std::string FinalDestination{};
	};

	struct PackagingProgress
	{
		float percent{ 0.0f };
		std::string message{};
	};

	class Packager
	{
	public:
		Packager(std::unique_ptr<PackageData> data, std::shared_ptr<ThreadPool> threadPool);
		~Packager();

		bool Completed() const;
		bool HasError() const;

		PackagingProgress GetProgress() const;

		void FinishPackaging();

	private:
		void RunPackager();
		void UpdateProgress(float percent, std::string_view message);
		std::string CreateConfigFile(const std::string& tempFilepath);
		std::string CreateAssetDefsFile(const std::string& tempFilepath, const rapidjson::Value& assets);
		std::vector<std::string> CreateSceneFiles(const std::string& tempFilepath, const rapidjson::Value& scenes);
		void CopyFilesToDestination();
		void CopyAssetsToDestination();

	private:
		std::unique_ptr<PackageData> m_PackageData;
		std::thread m_PackageThread;
		std::atomic_bool m_Packaging;
		std::atomic_bool m_HasError;
		mutable std::shared_mutex m_ProgressMutex;
		PackagingProgress m_Progress;

		std::shared_ptr<ThreadPool> m_ThreadPool;
	};

}
