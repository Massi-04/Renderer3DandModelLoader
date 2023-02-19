#include "Core.h"
#include "Window.h"
#include "D3D.h"

#include <DirectXMath.h>

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"

#include "FBXLoader.h"
#include "TextureLoader.h"

void InitApp();
void Update();

void InitImGui();
void ImGuiBegin();
void ImGuiRender();
void ImGuiEnd();

void RendererBeginScene();
void RendererRender();
void RendererEndScene();

#define VEC_TO_RAD(v) { DirectX::XMConvertToRadians(v.X), DirectX::XMConvertToRadians(v.Y), DirectX::XMConvertToRadians(v.Z) }
#define VEC_TO_XVEC(v) { v.X, v.Y, v.Z, 1.0f }

static float s_ClearColor[4] =
{
    .2f, .2f, 0.0f, 1.0f
};

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

    return new Mesh(&default_md);
}

static ID3D11Buffer* mvpBuffer;
Transform model;
Camera cam;
float cameraMoveSpeed = 2.0f;
float cameraScroolMultiplier = 100.0f;
float mouseSens = 0.2f;
float mouseX = 0.0f;
float mouseY = 0.0f;
float totalTime = 0.0f;
float deltaTime = 0.0f;

LARGE_INTEGER freq, startTicks, currentTicks;

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

void AddMesh(const char* fbxFilePath)
{
   /* std::vector<MeshData> meshData = LoadFbx(fbxFilePath);

    for (MeshData& md : meshData)
    {
        s_Meshes.push_back(new Mesh(&md));
    }*/
}

void ClearTextures()
{
    for (Texture* t : s_Textures)
        delete t;
    s_Textures = std::vector<Texture*>();

    for (Mesh* m : s_Meshes)
    {
        m->SetTexture(nullptr);
    }
}

void AddTexture(const char* texFilePath)
{
    s_Textures.push_back(LoadTexture(texFilePath));
}

void RenderMesh(Mesh* mesh)
{
    ID3D11Buffer* vb = mesh->GetVertexBuffer();
    ID3D11Buffer* ib = mesh->GetIndexBuffer();

    uint32_t vbStride = sizeof(Vertex);
    uint32_t vbOffset = 0;

    GContext->IASetVertexBuffers(0, 1, &vb, &vbStride, &vbOffset);
    GContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

    if (mesh->GetTexture())
    {
        ID3D11ShaderResourceView* tex[] = { mesh->GetTexture()->GetTextureViewHandle() };

        GContext->PSSetShaderResources(0, 1, tex);
    }

    GContext->DrawIndexed(mesh->GetIndexBufferCount(), 0, 0);

    ID3D11ShaderResourceView* tex2[] = { nullptr };

    GContext->PSSetShaderResources(0, 1, tex2);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{   
    GInstance = hInstance;

    SpawnWindow(hInstance, { 1600, 900, 0, false, false, "Finestrella" });

    InitD3D();

    //FbxLoaderInit();

    InitTimer();

    InitApp();

    InitImGui();

    while (GAppShouldRun)
    {
        PullEvents();

        double currentTime = GetTime();
        deltaTime = currentTime - totalTime;
        totalTime = currentTime;
        
        ImGuiBegin();

        Update();

        RendererBeginScene();
        RendererRender();

        ImGuiRender();
        ImGuiEnd();
        
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

    mvpBuffer = CreateBuffer(Dynamic, ConstBuffer, Write, sizeof(DirectX::XMMATRIX), 0);

    GContext->VSSetConstantBuffers(0, 1, &mvpBuffer);

    Mesh* defaultMesh = GetDefaultMesh();

    Texture* defaultTex = LoadTexture("assets/doom.jpg");

    defaultMesh->SetTexture(defaultTex);

    s_Meshes.push_back(defaultMesh);
    s_Textures.push_back(defaultTex);
}

Vec3 GetForwardVector(Vec3 rotation)
{
    rotation = VEC_TO_RAD(rotation);

    Vec3 forward;

    forward.X = cos(rotation.X) * sin(rotation.Y);
    forward.Y = -sin(rotation.X);
    forward.Z = cos(rotation.X) * cos(rotation.Y);

    return forward;
}

Vec3 GetRightVector(Vec3 rotation)
{
    rotation = VEC_TO_RAD(rotation);

    Vec3 right;

    right.X = cos(rotation.Y);
    right.Y = 0.0f;
    right.Z = -sin(rotation.Y);

    return right;
}

Vec3 GetUpVector(Vec3 rotation)
{
    DirectX::XMVECTOR up = DirectX::XMVector3Cross(VEC_TO_XVEC(GetForwardVector(rotation)), VEC_TO_XVEC(GetRightVector(rotation)));

    return { up.m128_f32[0], up.m128_f32[1], up.m128_f32[2] };
}

DirectX::XMMATRIX GetModelMatrix(const Transform& transform)
{
    return
    {
        DirectX::XMMatrixTranslation(transform.Location.X, transform.Location.Y, transform.Location.Z)
        *
        DirectX::XMMatrixRotationRollPitchYawFromVector(VEC_TO_RAD(transform.Rotation))
        *
        DirectX::XMMatrixScaling(transform.Scale.X, transform.Scale.Y, transform.Scale.Z)
    };
}

DirectX::XMMATRIX GetViewMatrix(Vec3 camLocation, Vec3 camRotation)
{
    auto defaultView = DirectX::XMMatrixLookAtLH(VEC_TO_XVEC(camLocation), VEC_TO_XVEC((GetForwardVector(camRotation) + camLocation)), VEC_TO_XVEC(GetUpVector(camRotation)));
    auto zRotation = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(camRotation.Z));

    return defaultView * zRotation;
}

DirectX::XMMATRIX GetPerspectiveMatrix(float aspectRatio, float fov)
{
    return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), aspectRatio, 0.1f, 1000.0f);
}

void MoveCameraForward(float direction)
{
    Vec3 forward = GetForwardVector(cam.Rotation);

    float finalSpeed = cameraMoveSpeed * deltaTime * direction;

    cam.Location += forward * finalSpeed;
}

void MoveCameraRight(float direction)
{
    Vec3 right = GetRightVector(cam.Rotation);

    float finalSpeed = cameraMoveSpeed * deltaTime * direction;

    cam.Location += right * finalSpeed;
}

void MoveCameraUp(float direction)
{
    Vec3 up = GetUpVector(cam.Rotation);

    float finalSpeed = cameraMoveSpeed * deltaTime * direction;

    cam.Location += up * finalSpeed;
}

void UpdateCameraLocation()
{
    if (ImGui::IsKeyDown(ImGuiKey_A))
    {
        MoveCameraRight(-1.0f);
    }
    if (ImGui::IsKeyDown(ImGuiKey_D))
    {
        MoveCameraRight(1.0f);
    }
    if (ImGui::IsKeyDown(ImGuiKey_Q))
    {
        MoveCameraUp(-1.0f);
    }
    if (ImGui::IsKeyDown(ImGuiKey_E))
    {
        MoveCameraUp(1.0f);
    }
    if (ImGui::IsKeyDown(ImGuiKey_S))
    {
        MoveCameraForward(-1.0f);
    }
    if (ImGui::IsKeyDown(ImGuiKey_W))
    {
        MoveCameraForward(1.0f);
    }
}

void UpdateCameraRotation()
{
    double newX, newY;
    newX = ImGui::GetMousePos().x;
    newY = ImGui::GetMousePos().y;

    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        float deltaX = mouseX - newX;
        float deltaY = mouseY - newY;

        if (deltaX != 0.0f)
        {
            cam.Rotation.Y += -deltaX * mouseSens;
        }
        if (deltaY != 0.0f)
        {
            cam.Rotation.X += -deltaY * mouseSens;
        }
    }

    mouseX = newX;
    mouseY = newY;
}

void Update()
{ 
    UpdateCameraLocation();
    UpdateCameraRotation();

    // update MVP
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
    for (Mesh* m : s_Meshes)
    {
        RenderMesh(m);
    }
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

#define IM_SUBMENU(MenuName, Code)\
if(ImGui::CollapsingHeader(MenuName, ImGuiTreeNodeFlags_DefaultOpen)) \
{ \
    ImGui::Spacing(); \
    ImGui::Spacing(); \
    Code \
}\
ImGui::Spacing(); \
ImGui::Spacing();

void TexturePropertyPanel(Texture* texture)
{
    ImGui::Text("Name: %s", texture->GetName().c_str());
    ImGui::Text("Width: %i", texture->GetWidth());
    ImGui::Text("Height: %i", texture->GetHeight());
    ImGui::Image((void*)texture->GetTextureViewHandle(), ImVec2(100, 100));
}

void MeshPropertyPanel(Mesh* mesh, const std::vector<Texture*>& availTextures)
{
    ImGui::Text("Name: %s", mesh->GetName().c_str());
    ImGui::Text("Poly count: %i", mesh->GetPolyCount());
    ImGui::Text("Vertices: %i", mesh->GetVertexBufferCount());
    ImGui::Text("Indices: %i", mesh->GetIndexBufferCount());

    Texture* currentlyBoundTexture = mesh->GetTexture();

    ImGui::Text("Currently bound texture: %s", currentlyBoundTexture ? currentlyBoundTexture->GetName().c_str() : "NULL");

    if (ImGui::Button("Remove texture binding"))
    {
        mesh->SetTexture(nullptr);
        currentlyBoundTexture = nullptr;
    }

    for (uint32_t i = 0; i < availTextures.size(); i++)
    {
        bool isBound = currentlyBoundTexture == availTextures[i];

        ImGui::PushID(i);
        if (ImGui::Selectable(availTextures[i]->GetName().c_str(), isBound) && !isBound)
        {
            mesh->SetTexture(availTextures[i]);
        }
        ImGui::PopID();
    }
}

void AssetsPanel(Mesh*& selectedMesh, Texture*& selectedTexture)
{
    IM_SUBMENU
    (
        "Mesh assets",
        if (ImGui::Button("Remove all meshes"))
        {
            ClearMeshes();
            selectedMesh = nullptr;
        }
        if (ImGui::Button("Add mesh"))
        {
            String filePath = OpenFileDialog("Autodesk FBX (.fbx)\0*.fbx*\0\0");
            if (!filePath.empty())
            {
                AddMesh(filePath.c_str());
            }
        }
        for (Mesh* m : s_Meshes)
        {
            ImGui::PushID(m);
            if (ImGui::Selectable(m->GetName().c_str(), selectedMesh == m))
            {
                selectedMesh = m;
            }
            ImGui::PopID();
        }
    );
    IM_SUBMENU
    (
        "Texture assets",
        if (ImGui::Button("Remove all textures"))
        {
            ClearTextures();
            selectedTexture = nullptr;
        }
        if (ImGui::Button("Add texture"))
        {
            String filePath = OpenFileDialog("Images (.jpg, .png, .jpeg)\0*.jpg;*.png;*.jpeg\0\0");
            if (!filePath.empty())
            {
                AddTexture(filePath.c_str());
            }
        }
        for (Texture* t : s_Textures)
        {
            ImGui::PushID(t);
            if (ImGui::Selectable(t->GetName().c_str(), selectedTexture == t))
            {
                selectedTexture = t;
            }
            ImGui::PopID();
        }
    );
}

const char* GetFillModeStr(EFillMode fillMode)
{
    switch (fillMode)
    {
        case Wireframe: return "Wireframe";
        case FillSolid: return "Solid";
        default:        return "Unknown ?!?";
    }
}

const char* GetCullModeStr(ECullMode cullMode)
{
    switch (cullMode)
    {
        case NoCull: return "None";
        case Front:  return "Front";
        case Back:   return "Back";
        default:     return "Unknown ?!?";
    }
}

void SceneSettings()
{
    IM_SUBMENU
    (
        "Boh",
        const WindowProps& wndProps = GetWndProps();
        ImGui::Text("Client size: %ix%i", wndProps.Width, wndProps.Height);
        ImGui::Text("(F11 to swap) Fullscreen state: %s", wndProps.Fullscreen ? "Fullscreen" : "Windowed");
    );

    IM_SUBMENU
    (
        "Renderer / Rasterizer",
        RasterizerDesc rd = GetCurrentRasterizerDesc();
        ImGui::Text("(F3 to swap) Fill mode: %s", GetFillModeStr(rd.FillMode));
        ImGui::Text("(F4 to swap) Cull mode: %s", GetCullModeStr(rd.CullMode));
    );

    IM_SUBMENU
    (
        "Camera",
        ImGui::DragFloat3("Cam location", &cam.Location.X, 0.1f);
        ImGui::DragFloat3("Cam rotation", &cam.Rotation.X, 0.1f);
        ImGui::DragFloat("Cam FOV", &cam.FOV, 0.1f);
        ImGui::DragFloat("Move speed", &cameraMoveSpeed, 0.01f);
        ImGui::DragFloat("Mouse sens", &mouseSens, 0.01f);
        ImGui::DragFloat("Scroll multiplier", &cameraScroolMultiplier, 0.01f);
    );

    IM_SUBMENU
    (
        "Model",
        ImGui::DragFloat3("Model location", &model.Location.X, 0.1f);
        ImGui::DragFloat3("Model rotation", &model.Rotation.X, 0.1f);
        ImGui::DragFloat3("Model scale", &model.Scale.X, 0.1f);
    );
}

void ImGuiRender()
{
    static Mesh* selectedMesh = nullptr;
    static Texture* selectedTexture = nullptr;

    ImGui::SetNextWindowPos({ 0.0f, 0.0f });
    ImGui::Begin("Scene settings");
    SceneSettings();
    ImGui::End();

    ImGui::Begin("Assets");
    AssetsPanel(selectedMesh, selectedTexture);
    ImGui::End();

    if (selectedMesh)
    {
        ImGui::Begin("Mesh properties");
        MeshPropertyPanel(selectedMesh, s_Textures);
        ImGui::End();
    }

    if (selectedTexture)
    {
        ImGui::Begin("Texture properties");
        TexturePropertyPanel(selectedTexture);
        ImGui::End();
    }
}

void ImGuiEnd()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}