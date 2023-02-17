#include "Core.h"

#include <commdlg.h>

HINSTANCE GInstance = nullptr;
HWND GWnd = nullptr;
bool GAppShouldRun = true;

void ShowAlert(const String& msg)
{
	MessageBoxA(GWnd, msg.c_str(), "Messaggio molto importante", MB_OK | MB_ICONEXCLAMATION);
}

String OpenFileDialog(const char* filters)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	CHAR currentDir[256] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = currentDir;
	ofn.lpstrFilter = filters;
	ofn.nFilterIndex = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn) == TRUE)
		return ofn.lpstrFile;

	return std::string();
}
