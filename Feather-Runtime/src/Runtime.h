#pragma once

#include <SDL.h>

namespace sol {
	class state;
}

namespace Feather {

	struct GameConfig;
	struct FAsset;
	class Window;
	enum class AssetType;

	class RuntimeApp
	{
	public:
		RuntimeApp();
		~RuntimeApp();

		void Run();

	private:
		void Initialize();

		bool LoadShaders();
		bool LoadConfig(sol::state& lua);
		bool LoadRegistryContext();
		void LoadBindings();
		bool LoadScripts();
		bool LoadPhysics();
		bool LoadZip();

		void ProcessEvents();
		void Update();
		void Render();

		void CleanUp();

	private:
		std::unique_ptr<Window> m_Window;
		std::unique_ptr<GameConfig> m_GameConfig;
		std::unordered_map<AssetType, std::vector<std::unique_ptr<FAsset>>> m_mapFAssets;
		SDL_Event m_Event;
		bool m_Running;
	};
}