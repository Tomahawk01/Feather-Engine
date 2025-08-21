#pragma once

#include "Button.h"
#include "GamepadButtons.h"
#include "Utils/SDL_Wrappers.h"

namespace Feather {

	class Gamepad
	{
	public:
		Gamepad(Controller controller);
		~Gamepad() = default;

		void Update();
		void OnButtonPressed(int btn);
		void OnButtonReleased(int btn);

		const bool IsButtonPressed(int btn) const;
		const bool IsButtonJustPressed(int btn) const;
		const bool IsButtonJustReleased(int btn) const;

		const bool IsGamepadPresent() const;

		const Sint16 GetAxisPosition(Uint8 axis);
		void SetAxisPositionValue(Uint8 axis, Sint16 value);

		inline void SetJoystickHatValue(Uint8 value) { m_JoystickHatValue = value; }
		inline const Uint8 GetJoystickHatValue() { return m_JoystickHatValue; }

		inline const bool CheckJoystickID(SDL_JoystickID joystickID) const { return m_InstanceID == joystickID; }
		inline const std::string& GetName() const { return m_Name; }

	private:
		Controller m_Controller;
		std::map<int, Button> m_Buttons;
		SDL_JoystickID m_InstanceID;
		std::map<Uint8, Sint16> m_AxisValues;
		Uint8 m_JoystickHatValue;
		std::string m_Name;
	};

}
