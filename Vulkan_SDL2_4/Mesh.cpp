#include "Mesh.h"

glm::vec3 Mesh::defaultColor = { 1.0f, 0.0f, 0.0f };
glm::vec2 Mesh::defaultTexCoord = { 0.0f, 0.0f };

bool Mesh::loadMesh(FbxNode * node)
{
	FbxMesh* mesh = node->GetMesh();
	if (!mesh)
		return false;

	int triangleCount = mesh->GetPolygonCount();
	vertices.reserve(3 * triangleCount);
	vertexType vertex;

	FbxVector4* ctrlPoints = mesh->GetControlPoints();

	for (int i = 0; i < triangleCount; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			int ctrlPointIndex = mesh->GetPolygonVertex(i, j);
			readVertices(mesh, ctrlPointIndex, vertex);
			vertex.color = { 1.0f, 0.0f, 0.0f };
			readTexCoord(mesh, i, j, vertex);
			vertices.push_back(vertex);
		}
	}
	//此处为了获得索引以减少空间使用
	//因一个顶点得纹理坐标可能有多个不同值
	//暂时注释掉

	/*std::vector<Vertex> uniqueVertices;
	std::unordered_map<glm::vec3, indexType, hashVec3> store;
	for (const auto& v : vertices)
	{
		if (store.count(v.pos) == 0)
		{
			store[v.pos] = uniqueVertices.size();
			uniqueVertices.push_back(v);
		}
		indices.push_back(store[v.pos]);
	}
	vertices = std::move(uniqueVertices);*/

	return true;
}

Mesh * Mesh::Create(FbxNode * node)
{
	Mesh* mesh = new Mesh;
	if (!mesh)
		return nullptr;
	if (mesh->loadMesh(node))
		return mesh;
	else
		return nullptr;
}

void Mesh::readVertices(FbxMesh * mesh, int ctrlPointIndex, vertexType& v)
{
	FbxVector4* ctrlPoints = mesh->GetControlPoints();
	v.pos.x = ctrlPoints[ctrlPointIndex][0];
	v.pos.y = ctrlPoints[ctrlPointIndex][1];
	v.pos.z = ctrlPoints[ctrlPointIndex][2];
}

void Mesh::readTexCoord(FbxMesh * mesh, int polyIndex, int vertexIndex, vertexType & v)
{
	int layerCount = mesh->GetUVLayerCount();
	if (!layerCount)
	{
		v.texCoord = defaultTexCoord;
		return;
	}

	FbxStringList UVSetNameList;
	// Get the name of each set of UV coords
	FbxVector2 coord;
	bool unmapped;
	mesh->GetUVSetNames(UVSetNameList);
	mesh->GetPolygonVertexUV(polyIndex, vertexIndex, UVSetNameList.GetStringAt(0), coord, unmapped);
	v.texCoord.x = coord[0];
	v.texCoord.y = coord[1];
}

const decltype(Mesh::vertices)::value_type * Mesh::getVertices() const
{
	return (this->vertices.data());
}

std::ostream & operator<<(std::ostream & os, const Vertex & v)
{
	os << "position : [ " << v.pos.x << ", " << v.pos.y << ", " << v.pos.z << " ]" << std::endl
		<< "texture : [ " << v.texCoord.x << ", " << v.texCoord.y << " ]";
	return os;
}
