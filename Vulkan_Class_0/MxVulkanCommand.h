#pragma once
#ifndef _MX_VULKAN_COMMAND_H_
#define _MX_VULKAN_COMMAND_H_

#include"MxVulkanManager.h"

#include<vector>
#include<list>
#include<initializer_list>

namespace Mixel
{
	class MxVulkanCommand
	{
	private:
		bool mIsReady;

		const MxVulkanManager* mManager;
		VkQueue mQueue;

		VkCommandPool mCommandPool;
		std::list<VkCommandBuffer> mCommandBuffers;

		static VkCommandBufferAllocateInfo sTempBufferAllocInfo;
		static VkCommandBufferBeginInfo sTempBufferBeginInfo;
		
	public:
		using CommandBufferIterator = std::list<VkCommandBuffer>::const_iterator;

		MxVulkanCommand();
		bool setup(const MxVulkanManager* manager);
		bool createCommandPool(VkQueueFlagBits queueType);
		CommandBufferIterator allocCommandBuffers(VkCommandBufferLevel level, uint32_t count);
		bool beginCommandBuffer(CommandBufferIterator it,VkCommandBufferUsageFlagBits usage= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		bool endCommandBuffer(CommandBufferIterator it);
		void freeCommandBuffers(const std::initializer_list<CommandBufferIterator>& its);
		VkCommandBuffer beginTempCommandBuffer();
		void endTempCommandBuffer(VkCommandBuffer commandBuffer);
		void destroy();
		~MxVulkanCommand();
	};
}
#endif // !_MX_VULKAN_COMMAND_H_