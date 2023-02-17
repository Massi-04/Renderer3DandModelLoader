#pragma once

#include "Core.h"
#include "Mesh.h"

void FbxLoaderInit();
void FbxLoaderShutdown();

std::vector<MeshData> LoadFbx(const char* filePath);