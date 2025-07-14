#pragma once

#include "IDisplay.h"

#include <string>
#include <functional>
#include <vector>

namespace Feather {

	class ProjectInfo;
	class CoreEngineData;
	class MainRegistry;

	class ProjectSettingsDisplay : public IDisplay
	{
	public:
		ProjectSettingsDisplay();
		~ProjectSettingsDisplay();

		virtual void Update() override;
		virtual void Draw() override;

	private:
		struct SettingItem
		{
			std::string name{};
			std::function<void()> drawFunc;
		};

		struct SettingCategory
		{
			std::string name;
			std::vector<SettingItem> items;
			std::vector<SettingCategory> subCategories;
		};

		void DrawCategoryTree(const SettingCategory& category, std::string& selected);
		void DrawSettingsPanel(const SettingCategory& category, std::string& selected);
		void CreateProjectSettings();

		SettingCategory CreateGeneralSettings(CoreEngineData& coreGlobals, ProjectInfo& projectInfo, MainRegistry& mainRegistry);
		SettingCategory CreatePhysicsSettings(CoreEngineData& coreGlobals, ProjectInfo& projectInfo, MainRegistry& mainRegistry);
		SettingCategory CreateGraphicsSettings(CoreEngineData& coreGlobals, ProjectInfo& projectInfo, MainRegistry& mainRegistry);
		SettingCategory CreateAudioSettings(CoreEngineData& coreGlobals, ProjectInfo& projectInfo, MainRegistry& mainRegistry);

	private:
		SettingCategory m_SettingsCategory;
		std::string m_SelectedCategory;
	};

}
