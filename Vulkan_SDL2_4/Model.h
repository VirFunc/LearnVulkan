#pragma once
#include<vulkan/vulkan.h>
#include<glm/glm.hpp>
#define FBXSDK_NAMESPACE_USING 0
#include<fbxsdk.h>
#include<iostream>
#include<string>
#include<vector>
#include<array>
#include<unordered_map>

#define MX_DEBUG

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;

	//����Vertex���ڵĶ����(VertexInputBinding)����
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0; //�������ݵİ󶨵�
		bindingDescription.stride = sizeof(Vertex); //����Vertex֮��ļ��
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	//���ض���������ÿ�� ���Ե����� 
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescription = {};
		attributeDescription[0].binding = 0; //�������ݵİ󶨵�
		attributeDescription[0].location = 0; //��vertex shader�е�location
		attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT; //���Ե����ݸ�ʽ
		attributeDescription[0].offset = offsetof(Vertex, pos); //���������һ��Vertex��ʼλ�õı�����

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, normal);

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
	std::vector<vertexType> mVertices;

	uint32_t mTriangleCount;

public:
	Mesh() = default;
	~Mesh() = default;

	uint32_t getTriangleCount() const;
	uint32_t getVertexCount() const;
	const Vertex* getBuffer() const; //���ر���Ķ�����������
	Vertex* getBuffer();
};


class Model
{
	friend class ModelImporter;
public:
	using meshType = Mesh;

private:
	std::vector<meshType> mMeshes;

public:
	Model() = default;
	~Model() = default;

	uint32_t getMeshCount() const;
	const Mesh* getBuffer() const;
	Mesh* getBuffer();
};

class ModelImporter
{
private:
	fbxsdk::FbxManager* mManager;
	bool mInitialized;

	void processNode(fbxsdk::FbxNode* node, Model& model);
	void processMesh(fbxsdk::FbxMesh* mesh, Model& model);
	void readPosition(fbxsdk::FbxVector4* ctrPoints, int ctrPointIndex, Vertex& v);
	void readUV(fbxsdk::FbxMesh* mesh, fbxsdk::FbxGeometryElementUV* uv, int ctrPointIndex, int polygonIndex, int vertexIndex, Vertex& v);
	void readNormal(fbxsdk::FbxMesh* mesh, fbxsdk::FbxGeometryElementNormal* normal, int vertexCount, Vertex& v);

public:
	ModelImporter();
	~ModelImporter();

	bool initialize();
	Model* loadModel(const std::string& path);
	void destroy();
};