#include "Core.h"
#include "Window.h"
#include "D3D.h"

void InitApp();
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

    InitApp();

    while (GAppShouldRun)
    {
        PullEvents();
        Update();
        Render();
    }

    return 0;
}

struct Vertex
{
    Vertex(Vec4 loc, Vec4 col)
    {
        Location = loc;
        Color = col;
    }

    Vec4 Location;
    Vec4 Color;
};

void InitApp()
{
    ID3DBlob* vertexShaderCode = CompileShader(L"assets/vertex.hlsl", VertexShader);
    ID3DBlob* pixelShaderCode = CompileShader(L"assets/pixel.hlsl", PixelShader);

    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        { "Location", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, Location), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "Color",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, Color),    D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ID3D11InputLayout* inputLayout;

    checkf(GDevice->CreateInputLayout(inputLayoutDesc, 2, vertexShaderCode->GetBufferPointer(), vertexShaderCode->GetBufferSize(), &inputLayout) == S_OK, "impossibile creare input layout");

    GContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    GContext->IASetInputLayout(inputLayout);

    Vertex vertexBufferData[] =
    {
        Vertex
        (
            { -0.5f, -0.5f, 0.0f, 1.0f },
            {  1.0f,  0.0f, 0.0f, 1.0f }
        ),
        Vertex
        (
            {  0.0f,  0.5f, 0.0f, 1.0f },
            {  0.0f,  1.0f, 0.0f, 1.0f }
        ),
        Vertex
        (
            {  0.5f, -0.5f, 0.0f, 1.0f },
            {  0.0f,  0.0f, 1.0f, 1.0f }
        )
    };

    uint32_t stride = sizeof(Vertex);
    uint32_t offset = 0;

    ID3D11Buffer* vertexBuffer = CreateBuffer(Dynamic, VertexBuffer, Write, sizeof(vertexBufferData), vertexBufferData);
    GContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    uint32_t indexBufferData[] =
    {
        0, 1, 2
    };

    ID3D11Buffer* indexBuffer = CreateBuffer(Default, IndexBuffer, None, sizeof(indexBufferData), indexBufferData);
    GContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    ID3D11VertexShader* vertexShader = CreateVertexShader(vertexShaderCode);
    ID3D11PixelShader* pixelShader = CreatePixelShader(pixelShaderCode);

    GContext->VSSetShader(vertexShader, nullptr, 0);
    GContext->PSSetShader(pixelShader, nullptr, 0);

    vertexShaderCode->Release();
    pixelShaderCode->Release();
}

void Update()
{
    static float change = 0.005f;
    static float col = 1.0f;
    
    if (col < 0.0f || col > 1.0f)
        change *= -1;

    col += change;

    s_ClearColor[2] = col / 1;
}

void Render()
{
    GContext->ClearRenderTargetView(GRenderTargetView, s_ClearColor);
    GContext->ClearDepthStencilView(GDepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    GContext->DrawIndexed(3, 0, 0);
    GSwapChain->Present(1, 0);
}