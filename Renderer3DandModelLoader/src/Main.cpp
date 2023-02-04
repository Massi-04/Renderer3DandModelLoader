#include "Core.h"
#include "Window.h"
#include "D3D.h"

void Update();
void Render();

static float s_ClearColor[4] =
{
    .2f, .2f, 0.0f, 1.0f
};

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{   
    GInstance = hInstance;

    SpawnWindow(hInstance, { 1600, 900, 0, false, false, "Finestrella" });

    InitD3D();

    while (GAppShouldRun)
    {
        PullEvents();
        Update();
        Render();
    }

    return 0;
}

void Update()
{
    static float change = 0.00005f;
    static float col = 1.0f;
    
    if (col < 0.0f || col > 1.0f)
        change *= -1;

    col += change;

    s_ClearColor[2] = col / 1;
}

void Render()
{
    GContext->ClearRenderTargetView(GRenderTargetView, s_ClearColor);
    GSwapChain->Present(0, 0);
}