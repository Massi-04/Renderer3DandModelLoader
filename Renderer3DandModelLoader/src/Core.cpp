#include "Core.h"

HINSTANCE GInstance = nullptr;
HWND GWnd = nullptr;
bool GAppShouldRun = true;

void ShowAlert(const String& msg)
{
	MessageBoxA(GWnd, msg.c_str(), "Messaggio molto importante", MB_OK | MB_ICONEXCLAMATION);
}
