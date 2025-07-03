#pragma once

#include <string>

namespace Feather {

	struct SaveProject
	{
		/* The name of the project */
		std::string projectName{};
		/* The active path of the project */
		std::string projectPath{};
		/* The active project file path */
		std::string projectFilePath{};
		/* The main lua script path of the project */
		std::string mainLuaScript{};
		/* Game exe file icon path */
		std::string fileIconPath{};
		/* The path to the script_list.lua file */
		std::string scriptListPath{};
	};

	struct GameConfig
	{
		std::string gameName{};
		std::string startupScene{};

		int windowWidth{ 640 };
		int windowHeight{ 480 };
		uint32_t windowFlags{ 0 };

		int cameraWidth{ 640 };
		int cameraHeight{ 480 };
		float cameraScale{ 1.0f };

		bool physicsEnabled{ false };
		int32_t positionIterations{ 8 };
		int32_t velocityIterations{ 8 };
		float gravity{ 9.8f };

		bool packageAssets{ false };

		void Reset()
		{
			gameName.clear();
			startupScene.clear();

			windowWidth = 640;
			windowHeight = 480;
			windowFlags = 0;

			cameraWidth = 640;
			cameraHeight = 480;
			cameraScale = 1.0f;

			physicsEnabled = false;
			positionIterations = 8;
			velocityIterations = 8;
			gravity = 9.8f;

			packageAssets = false;
		}
	};

}
