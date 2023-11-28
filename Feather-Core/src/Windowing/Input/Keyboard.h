#pragma once

#include <map>

#include "Keys.h"
#include "Button.h"

namespace Feather {

	class Keyboard
	{
	public:
		Keyboard();
		~Keyboard() = default;

		void Update();
		void OnKeyPressed(int key);
		void OnKeyReleased(int key);

		const bool IsKeyPressed(int key) const;
		const bool IsKeyJustPressed(int key) const;
		const bool IsKeyJustReleased(int key) const;

	private:
		std::map<int, Button> m_Buttons;
	};

}
