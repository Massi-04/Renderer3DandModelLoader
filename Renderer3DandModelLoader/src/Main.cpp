#include "Core.h"
#include "Window.h"
#include "D3D.h"

#include <DirectXMath.h>

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"

void InitApp();
void Update();

void InitImGui();
void ImGuiBegin();
void ImGuiRender();
void ImGuiEnd();

void RendererBeginScene();
void RendererRender();
void RendererEndScene();

struct Transform
{
    Vec3 Location;
    Vec3 Rotation;
    Vec3 Scale;
};

struct Camera
{
    Vec3 Location;
    Vec3 Rotation;
    float FOV;
};

static float s_ClearColor[4] =
{
    .2f, .2f, 0.0f, 1.0f
};

static ID3D11Buffer* mvpBuffer;
static Transform model;
static Camera cam;

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{   
    GInstance = hInstance;

    SpawnWindow(hInstance, { 1600, 900, 0, false, false, "Finestrella" });

    InitD3D();

    InitApp();

    InitImGui();

    while (GAppShouldRun)
    {
        PullEvents();
        
        Update();

        RendererBeginScene();
        RendererRender();
        
        ImGuiBegin();
        ImGuiRender();
        ImGuiEnd();
        
        RendererEndScene();
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
    // app

    model.Location = { 0.0f, 0.0f, 0.0f };
    model.Rotation = { 0.0f, 0.0f, 0.0f };
    model.Scale = { 1.0f, 1.0f, 1.0f };

    cam.Location = { 0.0f, 0.0f, -1.0f };
    cam.Rotation = { 0.0f, 0.0f, 0.0f };
    cam.FOV = 90;

    // d3d

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
            { -0.5f,  0.5f, 0.0f, 1.0f },
            {  0.0f,  1.0f, 0.0f, 1.0f }
        ),
        Vertex
        (
            {  0.5f,  0.5f, 0.0f, 1.0f },
            {  0.0f,  0.0f, 1.0f, 1.0f }
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
        0, 1, 2,
        0, 2, 3
    };

    ID3D11Buffer* indexBuffer = CreateBuffer(Default, IndexBuffer, None, sizeof(indexBufferData), indexBufferData);
    GContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    ID3D11VertexShader* vertexShader = CreateVertexShader(vertexShaderCode);
    ID3D11PixelShader* pixelShader = CreatePixelShader(pixelShaderCode);

    GContext->VSSetShader(vertexShader, nullptr, 0);
    GContext->PSSetShader(pixelShader, nullptr, 0);

    vertexShaderCode->Release();
    pixelShaderCode->Release();

    mvpBuffer = CreateBuffer(Dynamic, ConstBuffer, Write, sizeof(DirectX::XMMATRIX), 0);

    GContext->VSSetConstantBuffers(0, 1, &mvpBuffer);
}

DirectX::XMMATRIX GetModelMatrix(const Transform& transform)
{
    return
    {
        DirectX::XMMatrixScaling(transform.Scale.X, transform.Scale.Y, transform.Scale.Z)
        *
        DirectX::XMMatrixRotationRollPitchYawFromVector({ transform.Rotation.X, transform.Rotation.Y, transform.Rotation.Z, 0.0f })
        *
        DirectX::XMMatrixTranslation(transform.Location.X, transform.Location.Y, transform.Location.Z)
    };
}

DirectX::XMMATRIX GetViewMatrix(Vec3 camLocation, Vec3 camRotation)
{
    return
    {
        DirectX::XMMatrixRotationRollPitchYawFromVector({-camRotation.X, -camRotation.Y, -camRotation.Z, 0.0f})
        *
        DirectX::XMMatrixTranslation(-camLocation.X, -camLocation.Y, -camLocation.Z)
    };
}

DirectX::XMMATRIX GetPerspectiveMatrix(float aspectRatio, float fov)
{
    return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), aspectRatio, 0.1f, 100.0f);
}

void Update()
{
    model.Rotation.Z += 0.005f;
    
    auto mvp = 
        GetModelMatrix(model)
        * 
        GetViewMatrix(cam.Location, cam.Rotation)
        * 
        GetPerspectiveMatrix(GetWndAspectRatio(), cam.FOV);

    mvp = DirectX::XMMatrixTranspose(mvp);

    D3D11_MAPPED_SUBRESOURCE res = {};

    GContext->Map(mvpBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &mvp, sizeof(DirectX::XMMATRIX));

    GContext->Unmap(mvpBuffer, 0);
}

void RendererBeginScene()
{
    GContext->ClearRenderTargetView(GRenderTargetView, s_ClearColor);
    GContext->ClearDepthStencilView(GDepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RendererRender()
{
    GContext->DrawIndexed(6, 0, 0);
}

void RendererEndScene()
{
    GSwapChain->Present(1, 0);
}

void InitImGui()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    checkf(ImGui_ImplWin32_Init(GWnd), "impossibile inizializzare imgui per win32");
    checkf(ImGui_ImplDX11_Init(GDevice, GContext), "impossibile inizializzare imgui per dx11");
}

void ImGuiBegin()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiRender()
{
    ImGui::ShowDemoWindow();
}

void ImGuiEnd()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}