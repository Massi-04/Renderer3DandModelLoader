#include "Input.h"

static uint8_t* sKeyboardState;

void Input::Init()
{
	// no need to delete this, let the OS free this stuff when the program terminates
	sKeyboardState = new uint8_t[256];
	memset(sKeyboardState, 0, 256);
}

void Input::UpdateKeyTable()
{
	GetKeyboardState(sKeyboardState);
}

bool Input::IsKeyDown(EKeyCode key)
{
	return sKeyboardState[(int)key] & (((uint8_t)0b1) << 7);
}

Vec2 Input::GetMousePos()
{
	tagPOINT p = {};
	GetCursorPos(&p);
	return { (float)p.x, (float)p.y };
}
