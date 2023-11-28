#pragma once

#include <map>

#include "Button.h"
#include "MouseButton.h"

namespace Feather {

	class Mouse
	{
	public:
		Mouse() = default;
		~Mouse() = default;

		void Update();
		void OnButtonPressed(int btn);
		void OnButtonReleased(int btn);

		const bool IsButtonPressed(int btn) const;
		const bool IsButtonJustPressed(int btn) const;
		const bool IsButtonJustReleased(int btn) const;

		const std::tuple<int, int> GetMouseScreenPosition();

		inline void SetMouseWheelX(int wheel) { m_WheelX = wheel; }
		inline void SetMouseWheelY(int wheel) { m_WheelY = wheel; }
		inline void SetMouseMoving(bool moving) { m_MouseMoving = moving; }

		inline const int GetMouseWheelX() const { return m_WheelX; }
		inline const int GetMouseWheelY() const { return m_WheelY; }
		inline const bool IsMouseMoving() const { return m_MouseMoving; }

	private:
		std::map<int, Button> m_Buttons
		{
			{F_MOUSE_LEFT, Button{}},
			{F_MOUSE_MIDDLE, Button{}},
			{F_MOUSE_RIGHT, Button{}}
		};

		int m_X{ 0 }, m_Y{ 0 }, m_WheelX{ 0 }, m_WheelY{ 0 };
		bool m_MouseMoving{ false };
	};

}
