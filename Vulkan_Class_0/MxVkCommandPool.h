#pragma once
#ifndef _MX_VK_COMMAND_H_
#define _MX_VK_COMMAND_H_

#include"MxVkManager.h"

#include<vector>
#include<list>
#include<initializer_list>
#include<utility>

namespace Mixel
{
	class MxVkCommandPool
	{
	private:
		bool mIsReady;

		const MxVkManager* mManager;
		VkQueue mQueue;

		VkCommandPool mCommandPool;
		std::list<VkCommandBuffer> mCommandBuffers;

		static VkCommandBufferAllocateInfo sTempBufferAllocInfo;
		static VkCommandBufferBeginInfo sTempBufferBeginInfo;

	public:
		using CommandBufferIterator = std::list<VkCommandBuffer>::const_iterator;
		using CommandBufferRange = std::pair<CommandBufferIterator, CommandBufferIterator>;

		MxVkCommandPool();
		bool setup(const MxVkManager* manager);
		bool createCommandPool(VkQueueFlagBits queueType);
		CommandBufferRange allocCommandBuffers(VkCommandBufferLevel level, uint32_t count);
		bool beginCommandBuffer(CommandBufferIterator it, VkCommandBufferUsageFlagBits usage = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		bool endCommandBuffer(CommandBufferIterator it);
		void freeCommandBuffers(const std::initializer_list<CommandBufferIterator>& its);
		VkCommandBuffer beginTempCommandBuffer();
		void endTempCommandBuffer(VkCommandBuffer commandBuffer);
		void destroy();
		~MxVkCommandPool();
	};
}
#endif // !_MX_VK_COMMAND_H_
