#pragma once

#include <SDL.h>

namespace Feather {

	class Window;

	class Gui
	{
	public:
		Gui() = delete;

		static bool InitImGui(Window* window);
		static void BeginImGui();
		static void EndImGui(Window* window);
		static void ShowImGuiDemo();

	private:
		inline static bool m_Initialized{ false };
	};

}
