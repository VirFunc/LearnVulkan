#pragma once
#include<vulkan/vulkan.h>
#include<glm/glm.hpp>
#include<FBX/fbxsdk.h>

#include<iostream>
#include<string>
#include<vector>
#include<array>
#include<unordered_map>

#define MX_DEBUG

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	//返回Vertex对于的顶点绑定(VertexInputBinding)描述
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0; //顶点数据的绑定点
		bindingDescription.stride = sizeof(Vertex); //两个Vertex之间的间隔
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	//返回顶点数据中每个 属性的描述 
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescription = {};
		attributeDescription[0].binding = 0; //顶点数据的绑定点
		attributeDescription[0].location = 0; //在vertex shader中的location
		attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT; //属性的数据格式
		attributeDescription[0].offset = offsetof(Vertex, pos); //属性相对于一个Vertex起始位置的便宜量

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, color);

		attributeDescription[2].binding = 0;
		attributeDescription[2].location = 2;
		attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[2].offset = offsetof(Vertex, texCoord);

		return attributeDescription;
	}
};

class ModelImporter;

class Mesh
{
	friend class ModelImporter;
public:
	using vertexType = Vertex;

private:
	uint32_t mTriangleCount;
	std::vector<vertexType> mVertices;
	/*
	static glm::vec3 defaultColor;
	static glm::vec2 defaultTexCoord;

	struct hashVec3
	{
		std::hash<float> hashFloat;

		size_t operator()(const glm::vec3& v) const
		{
			return hashFloat(v.x) ^ hashFloat(v.y) ^ hashFloat(v.z);
		}
	};*/

public:

	Mesh() = default;
	~Mesh() = default;
	/*bool loadMesh(FbxNode* node);
	static Mesh* Create(FbxNode* node);
	void readVertices(FbxMesh * mesh, int ctrlPointIndex, vertexType& v);
	void readTexCoord(FbxMesh* mesh, int polyIndex, int vertexIndex, vertexType& v);*/

	/*const decltype(vertices)::value_type* getVertices() const;

	size_t getVertexCount() const
	{
		return vertices.size();
	}*/
};

//std::ostream& operator<<(std::ostream& os, const Vertex& v);

class Model
{
	friend class ModelImporter;
public:
	using meshType = Mesh;

private:
	std::vector<meshType> mMeshes;

public:
	
};

class ModelImporter
{
private:
	FbxManager* mManager;
	bool mInitialized;

	void processNode(FbxNode* node, Model& model);
	void processMesh(FbxMesh* mesh, Model& model);

public:
	ModelImporter();
	~ModelImporter();

	bool initialize();
	Model* loadModel(const std::string& path);
	void destroy();
};