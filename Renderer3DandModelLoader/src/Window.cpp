#include "Window.h"

#include "D3D.h"

#include "vendor/imgui/backends/imgui_impl_win32.h"

void OnWndResize(uint32_t, uint32_t);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
	case WM_SIZE:
		OnWndResize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CLOSE:
		GAppShouldRun = false;
		break;
	}

	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

static WindowProps sWndProps;

void SpawnWindow(HINSTANCE hInstance, const WindowProps& props)
{
	sWndProps = props;

	WNDCLASSEXA wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEXA);
	wndClass.hInstance = hInstance;
	wndClass.lpfnWndProc = WindowProc;
	wndClass.lpszClassName = "mhanz";

	checkf(RegisterClassExA(&wndClass) != 0, "impossibile registrare classe finestra!");

	GWnd = CreateWindowExA
	(
		0, "mhanz", props.Title.c_str(), WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, props.Width, props.Height, nullptr, nullptr, hInstance, nullptr
	);

	checkf(GWnd, "impossibile creare la finestra!");

	ShowWindow(GWnd, 1);
}

const WindowProps& GetWndProps()
{
	return sWndProps;
}

float GetWndAspectRatio()
{
	return (float)sWndProps.Width / sWndProps.Height;
}

void PullEvents()
{
	MSG msg;
	while (PeekMessageA(&msg, GWnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void OnWndResize(uint32_t newWidth, uint32_t newHeight)
{
	sWndProps.Width = newWidth;
	sWndProps.Height = newHeight;
	if(GD3DInitalized)
		ResizeFrameBuffer(newWidth, newHeight);
}