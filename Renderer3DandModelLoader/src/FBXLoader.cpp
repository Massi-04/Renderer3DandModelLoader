#include "FBXLoader.h"

#include "fbxsdk.h"

static FbxManager* s_SdkManager;

void FbxLoaderInit()
{
	s_SdkManager = FbxManager::Create();
}

void FbxLoaderShutdown()
{
	s_SdkManager->Destroy();
}

#define GET_IMPORTER_ERR(preErr, importer) ( String(preErr).append(importer->GetStatus().GetErrorString()) )

void GetMeshes(FbxNode* rootNode, std::vector<FbxMesh*>& meshes)
{
	if (rootNode->GetMesh())
	{
		meshes.push_back(rootNode->GetMesh());
	}

	for (uint32_t i = 0; i < rootNode->GetChildCount(); i++)
	{
		GetMeshes(rootNode->GetChild(i), meshes);
	}
}

Vec2 ReadUv(fbxsdk::FbxMesh* pFbxMesh, int vertexIndex, int uvIndex)
{
	fbxsdk::FbxLayerElementUV* pFbxLayerElementUV = pFbxMesh->GetLayer(0)->GetUVs();

	if (pFbxLayerElementUV == nullptr) {
		return { 0.0f, 0.0f };
	}

	switch (pFbxLayerElementUV->GetMappingMode()) {

	case FbxLayerElementUV::eByControlPoint:
	{
		switch (pFbxLayerElementUV->GetReferenceMode()) {

		case FbxLayerElementUV::eDirect:
		{
			fbxsdk::FbxVector2 fbxUv = pFbxLayerElementUV->GetDirectArray().GetAt(vertexIndex);

			return { (float)fbxUv[0], (float)fbxUv[1] };

			break;
		}

		case FbxLayerElementUV::eIndexToDirect:
		{
			int id = pFbxLayerElementUV->GetIndexArray().GetAt(vertexIndex);
			fbxsdk::FbxVector2 fbxUv = pFbxLayerElementUV->GetDirectArray().GetAt(id);

			return { (float)fbxUv[0], (float)fbxUv[1] };

			break;
		}
		}

		break;
	}


	case FbxLayerElementUV::eByPolygonVertex:
	{


		switch (pFbxLayerElementUV->GetReferenceMode()) {

			// Always enters this part for the example model
		case FbxLayerElementUV::eDirect:
		case FbxLayerElementUV::eIndexToDirect:
		{


			return { (float)pFbxLayerElementUV->GetDirectArray().GetAt(uvIndex).mData[0], (float)pFbxLayerElementUV->GetDirectArray().GetAt(uvIndex).mData[1] };

			break;
		}
		}

		break;
	}
	}

	return { 0.0f, 0.0f };
}

void FillMeshData(FbxMesh* fbxMeshObj, MeshData* outMeshData)
{
	checkf(fbxMeshObj && outMeshData, "null params passed ::GetMeshData");

	// populate vertex buffer

	bool sOk = fbxMeshObj->SplitPoints();

	FbxVector4* pts = fbxMeshObj->GetControlPoints();

	FbxLayerElementArrayTemplate<FbxVector2>* uvs;

	bool uvsOk = fbxMeshObj->GetTextureUV(&uvs);

	uint32_t polyCount = fbxMeshObj->GetPolygonCount();

	uint32_t controlPointIndex = 0;

	for (uint32_t i = 0; i < polyCount; i++)
	{
		uint32_t polySize = fbxMeshObj->GetPolygonSize(i);

		for (uint32_t j = 0; j < polySize; j++)
		{
			FbxVector4 vertex = fbxMeshObj->GetControlPointAt(controlPointIndex);

			Vec2 uv = ReadUv(fbxMeshObj, controlPointIndex, fbxMeshObj->GetTextureUVIndex(i, j));

			uv.Y = 1.0f - uv.Y;

			Vertex v;
			v.Location = { (float)vertex[0], (float)vertex[1], (float)vertex[2], 1.0f };
			v.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
			v.TextureCoords = uv;
			
			outMeshData->VertexBufferData.push_back(v);

			outMeshData->IndexBufferData.push_back(fbxMeshObj->GetPolygonVertex(i, j));

			controlPointIndex++;
		}

	}

	// mesh name and poly count

	outMeshData->Name = fbxMeshObj->GetName();
	outMeshData->PolyCount = fbxMeshObj->GetPolygonCount();
}

std::vector<MeshData> LoadFbx(const char* filePath)
{
	FbxImporter* importer = FbxImporter::Create(s_SdkManager, "");

	if (!importer->Initialize(filePath, -1, nullptr))
	{
		ShowAlert(GET_IMPORTER_ERR("Impossibile inizializzare l'importer: ", importer));
		return {};
	}

	FbxScene* scene = FbxScene::Create(s_SdkManager, "myScene");

	if(!importer->Import(scene))
	{
		ShowAlert(GET_IMPORTER_ERR("impossibile importare la scena: ", importer));
		return {};
	}

	FbxNode* root = scene->GetRootNode();

	FbxGeometryConverter converter(s_SdkManager);
	converter.Triangulate(scene, true);

	std::vector<FbxMesh*> meshes;

	for (uint32_t i = 0; i < root->GetChildCount(); i++)
	{
		GetMeshes(root->GetChild(i), meshes);
	}

	std::vector<MeshData> result;
	result.reserve(meshes.size());

	for (uint32_t i = 0; i < meshes.size(); i++)
	{
		result.emplace_back();
		FillMeshData(meshes[i], &result.back());
	}

	for (FbxMesh* m : meshes)
	{
		m->Destroy(true);
	}

	scene->Destroy(true);
	importer->Destroy(true);

	return result;
}
