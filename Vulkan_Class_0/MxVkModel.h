#pragma once
#ifndef _MX_VK_MODEL_H_
#define _MX_VK_MODEL_H_

#include"MxVkBuffer.h"
#include"MxVkManager.h"

#include<glm/glm.hpp>
#define FBXSDK_NAMESPACE_USING 0
#include<fbxsdk.h>
#include<iostream>
#include<string>
#include<vector>
#include<array>
#include<unordered_map>

namespace Mix
{
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

	class MxVkModelImporter;

	class MxVkMesh
	{
		friend class MxVkModelImporter;
	public:
		using vertexType = Vertex;

	private:
		std::vector<vertexType> mVertices;
		MxVkBuffer* mVertexBuffer;
		uint32_t mTriangleCount;

	public:
		MxVkMesh() = default;
		~MxVkMesh();

		uint32_t getTriangleCount() const;
		uint32_t getVertexCount() const;
		const vertexType* getBuffer() const; //���ر���Ķ�����������
		vertexType* getBuffer();
		const MxVkBuffer* getVertexBuffer() const;
		MxVkBuffer* getVertexBuffer();

		void createVertexBuffer(const MxVkManager* manager, MxVkCommandPool* commandPool);
		void draw(const VkCommandBuffer commandBuffer);
	};


	class MxVkModel
	{
		friend class MxVkModelImporter;
	private:
		std::vector<MxVkMesh> mMeshes;

	public:
		MxVkModel() = default;
		~MxVkModel() = default;

		uint32_t getMeshCount() const;
		const MxVkMesh* getMeshes() const;
		MxVkMesh* getMeshes();

		void createVertexBuffer(const MxVkManager* manager, MxVkCommandPool* commandPool);
		void draw(const VkCommandBuffer commandBuffer);
	};

	class MxVkModelImporter
	{
	private:
		fbxsdk::FbxManager* mManager;
		bool mInitialized;

		void processNode(fbxsdk::FbxNode* node, MxVkModel& model);
		void processMesh(fbxsdk::FbxMesh* mesh, MxVkModel& model);
		void readPosition(fbxsdk::FbxVector4* ctrPoints, int ctrPointIndex, Vertex& v);
		void readUV(fbxsdk::FbxMesh* mesh, fbxsdk::FbxGeometryElementUV* uv, int ctrPointIndex, int polygonIndex, int vertexIndex, Vertex& v);
		void readNormal(fbxsdk::FbxMesh* mesh, fbxsdk::FbxGeometryElementNormal* normal, int vertexCount, Vertex& v);

	public:
		MxVkModelImporter();
		~MxVkModelImporter();

		bool initialize();
		MxVkModel* loadModel(const std::string& path);
		void destroy();
	};
}

#endif