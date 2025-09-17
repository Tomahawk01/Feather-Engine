#include "Gamepad.h"

#include "Logger/Logger.h"

namespace Feather {

	Gamepad::Gamepad(Controller controller)
		: m_Controller{ std::move(controller) },
		m_Buttons{
			{F_GP_A, Button{}},
			{F_GP_B, Button{}},
			{F_GP_X, Button{}},
			{F_GP_Y, Button{}},
			{F_GP_BACK, Button{}},
			{F_GP_GUIDE, Button{}},
			{F_GP_START, Button{}},
			{F_GP_LSTICK, Button{}},
			{F_GP_RSTICK, Button{}},
			{F_GP_LSHOULDER, Button{}},
			{F_GP_RSHOULDER, Button{}},
			{F_GP_DPAD_UP, Button{}},
			{F_GP_DPAD_DOWN, Button{}},
			{F_GP_DPAD_LEFT, Button{}},
			{F_GP_DPAD_RIGHT, Button{}}
		},
		m_InstanceID{ -1 },
		m_AxisValues{
			{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}
		},
		m_JoystickHatValue{ F_GP_HAT_CENTERED }
	{
		SDL_Joystick* joystick = SDL_GameControllerGetJoystick(m_Controller.get());
		if (!m_Controller || !joystick)
			throw("Controller or joystick are nullptr!");

		m_InstanceID = SDL_JoystickInstanceID(joystick);
		m_Name = std::string{ SDL_JoystickName(joystick) };

		auto num_axis = SDL_JoystickNumAxes(joystick);
		auto num_balls = SDL_JoystickNumBalls(joystick);
		auto num_hats = SDL_JoystickNumHats(joystick);
		auto num_buttons = SDL_JoystickNumButtons(joystick);

		F_TRACE("Gamepad name: {0}", m_Name);
		F_TRACE("Gamepad id: {0}", m_InstanceID);
		F_TRACE("Gamepad num axes: {0}", num_axis);
		F_TRACE("Gamepad num balls: {0}", num_balls);
		F_TRACE("Gamepad num hats: {0}", num_hats);
		F_TRACE("Gamepad num buttons: {0}", num_buttons);
	}

	void Gamepad::Update()
	{
		for (auto& [btn, button] : m_Buttons)
			button.Reset();
	}

	void Gamepad::OnButtonPressed(int btn)
	{
		if (btn == GP_UNKNOWN)
		{
			F_ERROR("Gamepad button '{0}' is unknown!", btn);
			return;
		}

		auto btnItr = m_Buttons.find(btn);
		if (btnItr == m_Buttons.end())
		{
			F_ERROR("Gamepad button '{0}' does not exist!", btn);
			return;
		}

		btnItr->second.Update(true);
	}

	void Gamepad::OnButtonReleased(int btn)
	{
		if (btn == GP_UNKNOWN)
		{
			F_ERROR("Gamepad button '{0}' is unknown!", btn);
			return;
		}

		auto btnItr = m_Buttons.find(btn);
		if (btnItr == m_Buttons.end())
		{
			F_ERROR("Gamepad button '{0}' does not exist!", btn);
			return;
		}

		btnItr->second.Update(false);
	}

	const bool Gamepad::IsButtonPressed(int btn) const
	{
		if (btn == GP_UNKNOWN)
		{
			F_ERROR("Gamepad button '{0}' is unknown!", btn);
			return false;
		}

		auto btnItr = m_Buttons.find(btn);
		if (btnItr == m_Buttons.end())
		{
			F_ERROR("Gamepad button '{0}' does not exist!", btn);
			return false;
		}

		return btnItr->second.IsPressed;
	}

	const bool Gamepad::IsButtonJustPressed(int btn) const
	{
		if (btn == GP_UNKNOWN)
		{
			F_ERROR("Gamepad button '{0}' is unknown!", btn);
			return false;
		}

		auto btnItr = m_Buttons.find(btn);
		if (btnItr == m_Buttons.end())
		{
			F_ERROR("Gamepad button '{0}' does not exist!", btn);
			return false;
		}

		return btnItr->second.JustPressed;
	}

	const bool Gamepad::IsButtonJustReleased(int btn) const
	{
		if (btn == GP_UNKNOWN)
		{
			F_ERROR("Gamepad button '{0}' is unknown!", btn);
			return false;
		}

		auto btnItr = m_Buttons.find(btn);
		if (btnItr == m_Buttons.end())
		{
			F_ERROR("Gamepad button '{0}' does not exist!", btn);
			return false;
		}

		return btnItr->second.JustReleased;
	}

	const bool Gamepad::IsGamepadPresent() const
	{
		return m_Controller != nullptr && SDL_NumJoysticks() > 0;
	}

	const bool Gamepad::IsRumbleSupported() const
	{
		return m_Controller != nullptr ? SDL_GameControllerHasRumble(m_Controller.get()) : false;
	}

	const Sint16 Gamepad::GetAxisPosition(Uint8 axis)
	{
		auto axisItr = m_AxisValues.find(axis);
		if (axisItr == m_AxisValues.end())
		{
			F_ERROR("Axis '{0}' does not exist!", axis);
			return 0;
		}

		return axisItr->second;
	}

	void Gamepad::SetAxisPositionValue(Uint8 axis, Sint16 value)
	{
		auto axisItr = m_AxisValues.find(axis);
		if (axisItr == m_AxisValues.end())
		{
			F_ERROR("Axis '{0}' does not exist!", axis);
			return;
		}

		axisItr->second = value;
	}

	void Gamepad::RumbleController(Uint16 lowFrequencyRumble, Uint16 highFrequencyRumble, Uint32 durationMs)
	{
		if (SDL_GameControllerRumble(m_Controller.get(), lowFrequencyRumble, highFrequencyRumble, durationMs) == -1)
		{
			F_WARN("Rumble not supported for controller '{}': {}", m_InstanceID, SDL_GetError());
		}
	}

}
