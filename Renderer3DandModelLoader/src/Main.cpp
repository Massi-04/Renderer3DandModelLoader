#include "Core.h"
#include "Window.h"
#include "D3D.h"
#include "FMath.h"
#include "Input.h"
#include "Resource.h"
#include "Editor.h"

void InitApp();
void Update();

void RendererBeginScene();
void RendererRender();
void RendererEndScene();

static float s_ClearColor[4] =
{
    .2f, .2f, 0.0f, 1.0f
};

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

static ID3D11Buffer* mvpBuffer;
Transform model;
Camera cam;
float cameraMoveSpeed = 2.0f;
float mouseSens = 0.2f;
Vec2 mousePos = { 0.0f, 0.0f };
float totalTime = 0.0f;
float deltaTime = 0.0f;

LARGE_INTEGER freq, startTicks, currentTicks;

Mesh* GetDefaultMesh()
{
    Vertex vertexBufferData[] =
    {
        Vertex
        (
            { -0.5f, -0.5f, 0.0f, 1.0f },
            {  1.0f,  0.0f, 0.0f, 1.0f },
            {  0.0,   1.0f }
        ),
        Vertex
        (
            { -0.5f,  0.5f, 0.0f, 1.0f },
            {  0.0f,  1.0f, 0.0f, 1.0f },
            {  0.0f,  0.0f }
        ),
        Vertex
        (
            {  0.5f,  0.5f, 0.0f, 1.0f },
            {  0.0f,  0.0f, 1.0f, 1.0f },
            {  1.0f,  0.0f }
        ),
        Vertex
        (
            {  0.5f, -0.5f, 0.0f, 1.0f },
            {  0.0f,  0.0f, 1.0f, 1.0f },
            {  1.0f,  1.0f }
        )
    };

    uint32_t indexBufferData[] =
    {
        0, 1, 2,
        0, 2, 3
    };

    MeshData default_md;
    default_md.Name = "Default Quad";
    default_md.PolyCount = 2;
    default_md.VertexBufferData = std::vector<Vertex>(4);
    default_md.IndexBufferData = std::vector<uint32_t>(6);
    memcpy(default_md.VertexBufferData.data(), vertexBufferData, sizeof(vertexBufferData));
    memcpy(default_md.IndexBufferData.data(), indexBufferData, sizeof(indexBufferData));

    return new Mesh("Default Quad", { default_md });
}

void InitTimer()
{
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&startTicks);
}

double GetTime()
{
    QueryPerformanceCounter(&currentTicks);
    return ((double)currentTicks.QuadPart - (double)startTicks.QuadPart) / freq.QuadPart;
}

std::vector<Mesh*> s_Meshes;
std::vector<Texture*> s_Textures;

void ClearMeshes()
{
    GContext->IASetVertexBuffers(0, 0, 0, nullptr, nullptr);
    GContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

    for (Mesh* m : s_Meshes)
        delete m;
    s_Meshes = std::vector<Mesh*>();
}

void AddMesh(const char* filePath)
{
    s_Meshes.push_back(Resource::Load<Mesh>(filePath));
}

void ClearTextures()
{
    for (Texture* t : s_Textures)
        delete t;
    s_Textures = std::vector<Texture*>();

    for (Mesh* m : s_Meshes)
    {
        m->SetTextureForAllSubmeshes(nullptr);
    }
}

void AddTexture(const char* texFilePath)
{
    s_Textures.push_back(Resource::Load<Texture>(texFilePath));
}

void RenderMesh(Mesh* mesh)
{
    for (Submesh* sub : mesh->GetSubmeshes())
    {
        ID3D11Buffer* vb = sub->GetVertexBuffer();
        ID3D11Buffer* ib = sub->GetIndexBuffer();

        uint32_t vbStride = sizeof(Vertex);
        uint32_t vbOffset = 0;

        GContext->IASetVertexBuffers(0, 1, &vb, &vbStride, &vbOffset);
        GContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

        ID3D11ShaderResourceView* tex[1] = { nullptr };

        if (sub->Texture)
        {
            tex[0] = sub->Texture->GetTextureViewHandle();
        }

        GContext->PSSetShaderResources(0, 1, tex);

        GContext->DrawIndexed(sub->GetProps().IndexCount, 0, 0);
    }
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{   
    GInstance = hInstance;

    SpawnWindow(hInstance, 
    {
        1600, 900,   // width, height
        0,           // refreshrate 0 = max
        false,       // fullscreen
        true,        // v-sync
        "Renderer3D" // title
    });

    InitD3D();

    InitTimer();

    Input::Init();

    InitApp();

    Editor::Init();

    while (GAppShouldRun)
    {   
        Input::UpdateKeyTable();
        
        PullEvents();

        double currentTime = GetTime();
        deltaTime = currentTime - totalTime;
        totalTime = currentTime;
        
        Editor::Begin();

        Update();

        RendererBeginScene();
        RendererRender();

        Editor::Render();
        Editor::End();
        
        RendererEndScene();
    }

    return 0;
}

void InitApp()
{
    // app

    model.Location = { 0.0f, 0.0f, 0.0f };
    model.Rotation = { 0.0f, 0.0f, 0.0f };
    model.Scale = { 1.0f, 1.0f, 1.0f };

    cam.Location = { 0.0f, 0.0f, -1.0f };
    cam.Rotation = { 0.0f, 0.0f, 0.0f };
    cam.FOV = 60;

    // d3d

    ID3DBlob* vertexShaderCode = CompileShader(L"assets/vertex.hlsl", VertexShader);
    ID3DBlob* pixelShaderCode = CompileShader(L"assets/pixel.hlsl", PixelShader);

    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        { "Location", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, Location), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "Color",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, Color),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TextureCoords", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, TextureCoords), D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ID3D11InputLayout* inputLayout;

    checkf(GDevice->CreateInputLayout(inputLayoutDesc, 3, vertexShaderCode->GetBufferPointer(), vertexShaderCode->GetBufferSize(), &inputLayout) == S_OK, "impossibile creare input layout");

    ID3D11VertexShader* vertexShader = CreateVertexShader(vertexShaderCode);
    ID3D11PixelShader* pixelShader = CreatePixelShader(pixelShaderCode);

    GContext->VSSetShader(vertexShader, nullptr, 0);
    GContext->PSSetShader(pixelShader, nullptr, 0);

    vertexShaderCode->Release();
    pixelShaderCode->Release();

    GContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    GContext->IASetInputLayout(inputLayout);

    mvpBuffer = CreateBuffer(Dynamic, ConstBuffer, Write, sizeof(Mat4), 0);

    GContext->VSSetConstantBuffers(0, 1, &mvpBuffer);

    Mesh* defaultMesh = GetDefaultMesh();

    Texture* defaultTex = Resource::Load<Texture>("assets\\doom.jpg");

    defaultMesh->SetTextureForAllSubmeshes(defaultTex);

    s_Meshes.push_back(defaultMesh);
    s_Textures.push_back(defaultTex);
}

void MoveCameraForward(float direction)
{
    Vec3 forward = FMath::GetForwardVector(cam.Rotation);

    float finalSpeed = cameraMoveSpeed * deltaTime * direction;

    cam.Location += forward * finalSpeed;
}

void MoveCameraRight(float direction)
{
    Vec3 right = FMath::GetRightVector(cam.Rotation);

    float finalSpeed = cameraMoveSpeed * deltaTime * direction;

    cam.Location += right * finalSpeed;
}

void MoveCameraUp(float direction)
{
    Vec3 up = FMath::GetUpVector(cam.Rotation);

    float finalSpeed = cameraMoveSpeed * deltaTime * direction;

    cam.Location += up * finalSpeed;
}

void UpdateCameraLocation()
{
    if (Input::IsKeyDown(EKeyCode::A))
    {
        MoveCameraRight(-1.0f);
    }
    if (Input::IsKeyDown(EKeyCode::D))
    {
        MoveCameraRight(1.0f);
    }
    if (Input::IsKeyDown(EKeyCode::Q))
    {
        MoveCameraUp(-1.0f);
    }
    if (Input::IsKeyDown(EKeyCode::E))
    {
        MoveCameraUp(1.0f);
    }
    if (Input::IsKeyDown(EKeyCode::S))
    {
        MoveCameraForward(-1.0f);
    }
    if (Input::IsKeyDown(EKeyCode::W))
    {
        MoveCameraForward(1.0f);
    }
}

void UpdateCameraRotation()
{
    Vec2 newMousePos = Input::GetMousePos();

    if (Input::IsKeyDown(EKeyCode::RightMouseBtn))
    {
        float deltaX = mousePos.X - newMousePos.X;
        float deltaY = mousePos.Y - newMousePos.Y;

        if (deltaX != 0.0f)
        {
            cam.Rotation.Y += -deltaX * mouseSens;
        }
        if (deltaY != 0.0f)
        {
            cam.Rotation.X += -deltaY * mouseSens;
        }
    }

    mousePos = newMousePos;
}

void Update()
{ 
    UpdateCameraLocation();
    UpdateCameraRotation();

    // update MVP
    auto mvp = 
        FMath::GetModelMatrix(model.Location, model.Rotation, model.Scale)
        *
        FMath::GetViewMatrix(cam.Location, cam.Rotation)
        *
        FMath::GetPerspectiveMatrix(GetWndAspectRatio(), cam.FOV, 0.1f, 1000.0f);

    mvp = FMath::GetMatrixTransposed(mvp);

    D3D11_MAPPED_SUBRESOURCE res = {};

    GContext->Map(mvpBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, &mvp, sizeof(Mat4));

    GContext->Unmap(mvpBuffer, 0);
}

void RendererBeginScene()
{
    GContext->ClearRenderTargetView(GRenderTargetView, s_ClearColor);
    GContext->ClearDepthStencilView(GDepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RendererRender()
{
    for (Mesh* m : s_Meshes)
    {
        RenderMesh(m);
    }
}

void RendererEndScene()
{
    GSwapChain->Present(GetWndProps().VSync, 0);
}