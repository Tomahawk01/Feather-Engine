#pragma once

#include "IDisplay.h"

#include <memory>
#include <string>

namespace Feather {

	struct GameConfig;

	class PackageGameDisplay : public IDisplay
	{
	public:
		PackageGameDisplay();
		~PackageGameDisplay();

		virtual void Update() override;
		virtual void Draw() override;

	private:
		std::unique_ptr<GameConfig> m_GameConfig;
		std::string m_DestinationPath;
		std::string m_ScriptListPath;
		std::string m_FileIconPath;

		bool m_Resizable;
		bool m_Borderless;
		bool m_FullScreen;
		bool m_Titlebar;

		bool m_ScriptListExist;
	};

}
