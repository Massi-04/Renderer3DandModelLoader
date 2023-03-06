#include "Editor.h"

#include "D3D.h"
#include "Window.h"
#include "Texture.h"
#include "Mesh.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"

void Editor::Init()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    checkf(ImGui_ImplWin32_Init(GWnd), "impossibile inizializzare imgui per win32");
    checkf(ImGui_ImplDX11_Init(GDevice, GContext), "impossibile inizializzare imgui per dx11");
}

void Editor::Begin()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Editor::End()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void TexturePropertyPanel(Texture* texture)
{
    ImGui::Text("Name: %s", texture->GetName().c_str());
    ImGui::Text("Width: %i", texture->GetWidth());
    ImGui::Text("Height: %i", texture->GetHeight());
    ImGui::Image((void*)texture->GetTextureViewHandle(), ImVec2(100, 100));
}
void MeshPropertyPanel(Mesh* mesh, const std::vector<Texture*>& availTextures)
{
    ImGui::Text("Name: %s", mesh->GetProps().Name.c_str());
    ImGui::Text("Poly count: %i", mesh->GetProps().PolyCount);
    ImGui::Text("Vertices: %i", mesh->GetProps().VertexCount);
    ImGui::Text("Indices: %i", mesh->GetProps().IndexCount);

    Texture* currentlyBoundTexture = mesh->GetTexture();

    ImGui::Text("Currently bound texture: %s", currentlyBoundTexture ? currentlyBoundTexture->GetName().c_str() : "NULL");

    if (ImGui::Button("Remove texture binding"))
    {
        mesh->SetTextureForAllSubmeshes(nullptr);
        currentlyBoundTexture = nullptr;
    }

    for (uint32_t i = 0; i < availTextures.size(); i++)
    {
        bool isBound = currentlyBoundTexture == availTextures[i];

        ImGui::PushID(i);
        if (ImGui::Selectable(availTextures[i]->GetName().c_str(), isBound) && !isBound)
        {
            mesh->SetTextureForAllSubmeshes(availTextures[i]);
        }
        ImGui::PopID();
    }
}

#pragma region TerribleExternBlock
extern std::vector<Mesh*> s_Meshes;
extern std::vector<Texture*> s_Textures;
extern void ClearMeshes();
extern void AddMesh(const char* filePath);
extern void ClearTextures();
extern void AddTexture(const char* texFilePath);
extern float deltaTime;
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
extern Transform model;
extern Camera cam;
extern float cameraMoveSpeed;
extern float mouseSens;
#pragma endregion

#define IM_SUBMENU(MenuName, Code)\
if(ImGui::CollapsingHeader(MenuName, ImGuiTreeNodeFlags_DefaultOpen)) \
{ \
    ImGui::Spacing(); \
    ImGui::Spacing(); \
    Code \
}\
ImGui::Spacing(); \
ImGui::Spacing();

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
            String filePath = OpenFileDialog("Obj (.obj)\0*.obj*\0\0");
            if (!filePath.empty())
            {
                AddMesh(filePath.c_str());
            }
        }
        for (Mesh* m : s_Meshes)
        {
            ImGui::PushID(m);
            if (ImGui::Selectable(m->GetProps().Name.c_str(), selectedMesh == m))
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
void SceneSettings()
{
    IM_SUBMENU
    (
        "Stats",
        const WindowProps & wndProps = GetWndProps();
        ImGui::Text("Client size: %ix%i", wndProps.Width, wndProps.Height);
        ImGui::Text("(F11 to swap) Fullscreen state: %s", wndProps.Fullscreen ? "Fullscreen" : "Windowed");
        ImGui::Text("FPS: %i %.3fms", int(1.0f / deltaTime), deltaTime * 1000.0f);
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
    );

    IM_SUBMENU
    (
        "Model",
        ImGui::DragFloat3("Model location", &model.Location.X, 0.1f);
        ImGui::DragFloat3("Model rotation", &model.Rotation.X, 0.1f);
        ImGui::DragFloat3("Model scale", &model.Scale.X, 0.1f);
    );
}

void Editor::Render()
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