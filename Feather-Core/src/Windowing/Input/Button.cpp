#include "Button.h"

void Feather::Button::Update(bool pressed)
{
	JustPressed = !IsPressed && pressed;
	JustReleased = IsPressed && !pressed;
	IsPressed = pressed;
}

void Feather::Button::Reset()
{
	JustPressed = false;
	JustReleased = false;
}


