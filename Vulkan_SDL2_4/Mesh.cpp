#include "Mesh.h"

//glm::vec3 Mesh::defaultColor = { 1.0f, 0.0f, 0.0f };
//glm::vec2 Mesh::defaultTexCoord = { 0.0f, 0.0f };
//
//bool Mesh::loadMesh(FbxNode * node)
//{
//	FbxMesh* mesh = node->GetMesh();
//	if (!mesh)
//		return false;
//
//	int triangleCount = mesh->GetPolygonCount();
//	vertices.reserve(3 * triangleCount);
//	vertexType vertex;
//
//	FbxVector4* ctrlPoints = mesh->GetControlPoints();
//
//	for (int i = 0; i < triangleCount; ++i)
//	{
//		for (int j = 0; j < 3; ++j)
//		{
//			int ctrlPointIndex = mesh->GetPolygonVertex(i, j);
//			readVertices(mesh, ctrlPointIndex, vertex);
//			vertex.color = { 1.0f, 0.0f, 0.0f };
//			readTexCoord(mesh, i, j, vertex);
//			vertices.push_back(vertex);
//		}
//	}
//	//此处为了获得索引以减少空间使用
//	//因一个顶点得纹理坐标可能有多个不同值
//	//暂时注释掉
//
//	/*std::vector<Vertex> uniqueVertices;
//	std::unordered_map<glm::vec3, indexType, hashVec3> store;
//	for (const auto& v : vertices)
//	{
//		if (store.count(v.pos) == 0)
//		{
//			store[v.pos] = uniqueVertices.size();
//			uniqueVertices.push_back(v);
//		}
//		indices.push_back(store[v.pos]);
//	}
//	vertices = std::move(uniqueVertices);*/
//
//	return true;
//}
//
//Mesh * Mesh::Create(FbxNode * node)
//{
//	Mesh* mesh = new Mesh;
//	if (!mesh)
//		return nullptr;
//	if (mesh->loadMesh(node))
//		return mesh;
//	else
//		return nullptr;
//}
//
//void Mesh::readVertices(FbxMesh * mesh, int ctrlPointIndex, vertexType& v)
//{
//	FbxVector4* ctrlPoints = mesh->GetControlPoints();
//	v.pos.x = ctrlPoints[ctrlPointIndex][0];
//	v.pos.y = ctrlPoints[ctrlPointIndex][1];
//	v.pos.z = ctrlPoints[ctrlPointIndex][2];
//}
//
//void Mesh::readTexCoord(FbxMesh * mesh, int polyIndex, int vertexIndex, vertexType & v)
//{
//	int layerCount = mesh->GetUVLayerCount();
//	if (!layerCount)
//	{
//		v.texCoord = defaultTexCoord;
//		return;
//	}
//
//	FbxStringList UVSetNameList;
//	// Get the name of each set of UV coords
//	FbxVector2 coord;
//	bool unmapped;
//	mesh->GetUVSetNames(UVSetNameList);
//	mesh->GetPolygonVertexUV(polyIndex, vertexIndex, UVSetNameList.GetStringAt(0), coord, unmapped);
//	v.texCoord.x = coord[0];
//	v.texCoord.y = coord[1];
//}
//
//const decltype(Mesh::vertices)::value_type * Mesh::getVertices() const
//{
//	return (this->vertices.data());
//}
//
//std::ostream & operator<<(std::ostream & os, const Vertex & v)
//{
//	os << "position : [ " << v.pos.x << ", " << v.pos.y << ", " << v.pos.z << " ]" << std::endl
//		<< "texture : [ " << v.texCoord.x << ", " << v.texCoord.y << " ]";
//	return os;
//}

void ModelImporter::processNode(FbxNode * node, Model & model)
{
	if (!(node->GetNodeAttribute()))
		return;

	switch (node->GetNodeAttribute()->GetAttributeType())
	{
	case FbxNodeAttribute::eMesh:
		processMesh(node->GetMesh(), model);
		break;
	default:
		break;
	}

	for (int i = 0; i < node->GetChildCount(); ++i)
	{
		processNode(node->GetChild(i), model);
	}
}

void ModelImporter::processMesh(FbxMesh * mesh, Model & model)
{
	if (!mesh)
		return;

	Mesh m;
	m.mTriangleCount = mesh->GetPolygonCount();
}

ModelImporter::ModelImporter() :mManager(nullptr), mInitialized(false)
{

}

ModelImporter::~ModelImporter()
{
	destroy();
}

bool ModelImporter::initialize()
{
	if (mManager)
		return true;

	mManager = FbxManager::Create();
	if (mManager)
		return true;
	else
		return false;
}

Model * ModelImporter::loadModel(const std::string & path)
{
	if (!mInitialized)
	{
		std::cerr << "Error : ModelImporter been used without initialization!" << std::endl;
		return nullptr;
	}

	FbxIOSettings* iosettings = FbxIOSettings::Create(mManager, IOSROOT);
	if (!iosettings)
	{
		std::cerr << "Error : Failed to allocate FbxIOSettiongs!" << std::endl;
		return nullptr;
	}

	mManager->SetIOSettings(iosettings);

	FbxScene* scene = FbxScene::Create(mManager, "MyScene");
	if (!scene)
	{
		std::cerr << "Error : Failed to allocate FbxScene!" << std::endl;
		return nullptr;
	}

	FbxImporter* importer = FbxImporter::Create(mManager, "");
	if (!importer)
	{
		std::cerr << "Error : Failed to allocate FbxImporter!" << std::endl;
		return nullptr;
	}

	//load moedl
	if (!importer->Initialize(path.c_str(), -1, mManager->GetIOSettings()))
	{
		std::cerr << "Warning : FbxImporter :" << std::endl
			<< importer->GetStatus().GetErrorString() << std::endl;
		return nullptr;
	}

#ifdef MX_DEBUG
	std::cout << "File name : " << importer->GetFileName() << std::endl;
	int major, minor, revision;
	importer->GetFileVersion(major, minor, revision);
	std::cout << "File version : " << major << "." << minor << "." << revision << std::endl;
#endif

	importer->Import(scene);
	importer->Destroy();

	Model* model = new Model;
	if (!model)
	{
		std::cerr << "Error : Failed to allocate Model!" << std::endl;
		return nullptr;
	}

	processNode(scene->GetRootNode(), *model);
	return model;
}

void ModelImporter::destroy()
{
	if (mManager)
	{
		mManager->Destroy();
		mManager = nullptr;
	}
}
