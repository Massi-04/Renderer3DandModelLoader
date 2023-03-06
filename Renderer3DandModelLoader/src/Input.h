#pragma once

#include "Core.h"

enum class EKeyCode
{
	A = 65,
	B, C, D, E, F,
	G, H, I, J, K,
	L, M, N, O, P,
	Q, R, S, T, U,
	V, W, X, Y, Z,

	LeftMouseBtn = VK_LBUTTON,
	RightMouseBtn = VK_RBUTTON,

	Spacebar = VK_SPACE
};

namespace Input
{
	void Init();
	void UpdateKeyTable();

	bool IsKeyDown(EKeyCode key);
	Vec2 GetMousePos();
};