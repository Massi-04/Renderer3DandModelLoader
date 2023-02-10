#pragma once

#include "Core.h"

struct WindowProps
{
	int32_t Width, Height, Refreshrate;
	bool Fullscreen, VSync;
	String Title;
};

void SpawnWindow(HINSTANCE hInstance, const WindowProps& props);
const WindowProps& GetWndProps();
float GetWndAspectRatio();

void PullEvents();