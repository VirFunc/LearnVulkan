#include "MxVulkanCommand.h"
namespace Mixel
{
	MxVulkanCommand::MxVulkanCommand() :mIsReady(false), mManager(nullptr), mCommandPool(VK_NULL_HANDLE)
	{
	}

	VkCommandBufferAllocateInfo MxVulkanCommand::sTempBufferAllocInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr, VK_NULL_HANDLE, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1
	};

	VkCommandBufferBeginInfo MxVulkanCommand::sTempBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr
	};

	bool MxVulkanCommand::setup(const MxVulkanManager * manager)
	{
		if (mIsReady)
			destroy();

		mManager = manager;
		mIsReady = true;
		return true;
	}

	bool MxVulkanCommand::createCommandPool(VkQueueFlagBits queueType)
	{
		if (!mIsReady)
			return false;

		VkCommandPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

		if (queueType == VK_QUEUE_GRAPHICS_BIT)
		{
			createInfo.queueFamilyIndex = mManager->getQueueFamilyIndices().graphics;
			mQueue = mManager->getQueue().graphics;
		}
		if (queueType == VK_QUEUE_COMPUTE_BIT)
		{
			createInfo.queueFamilyIndex = mManager->getQueueFamilyIndices().compute;
			mQueue = mManager->getQueue().compute;
		}

		createInfo.flags = 0;
		MX_VK_CHECK_RESULT(vkCreateCommandPool(mManager->getDevice(), &createInfo, nullptr, &mCommandPool));
		return true;
	}

	MxVulkanCommand::CommandBufferIterator MxVulkanCommand::allocCommandBuffers(VkCommandBufferLevel level, uint32_t count)
	{
		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = mCommandPool;
		allocateInfo.level = level;
		allocateInfo.commandBufferCount = count;

		std::vector<VkCommandBuffer> buffer(count);
		MX_VK_CHECK_RESULT(vkAllocateCommandBuffers(mManager->getDevice(), &allocateInfo, buffer.data()));
		auto it = mCommandBuffers.cend();
		mCommandBuffers.insert(mCommandBuffers.end(), buffer.cbegin(), buffer.cend());
		return it;
	}

	bool MxVulkanCommand::beginCommandBuffer(CommandBufferIterator it, VkCommandBufferUsageFlagBits usage)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = usage;

		MX_VK_CHECK_RESULT(vkBeginCommandBuffer(*it, &beginInfo));
		return true;
	}

	bool MxVulkanCommand::endCommandBuffer(CommandBufferIterator it)
	{
		MX_VK_CHECK_RESULT(vkEndCommandBuffer(*it));
		return true;
	}

	void MxVulkanCommand::freeCommandBuffers(const std::initializer_list<CommandBufferIterator>& its)
	{
		std::vector<VkCommandBuffer> buffers;
		buffers.reserve(its.size());
		auto it = its.begin();
		for (auto& buffer : buffers)
		{
			buffer = **it;
			mCommandBuffers.erase(*it);
		}
		vkFreeCommandBuffers(mManager->getDevice(), mCommandPool, buffers.size(), buffers.data());
	}

	VkCommandBuffer MxVulkanCommand::beginTempCommandBuffer()
	{
		sTempBufferAllocInfo.commandPool = mCommandPool;
		VkCommandBuffer temp;

		MX_VK_CHECK_RESULT(vkAllocateCommandBuffers(mManager->getDevice(), &sTempBufferAllocInfo, &temp));

		MX_VK_CHECK_RESULT(vkBeginCommandBuffer(temp, &sTempBufferBeginInfo));
		return temp;
	}

	void MxVulkanCommand::endTempCommandBuffer(VkCommandBuffer commandBuffer)
	{
		MX_VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.commandBufferCount = 1;
		vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(mQueue);
	}

	void MxVulkanCommand::destroy()
	{
		if (!mIsReady)
			return;

		vkDestroyCommandPool(mManager->getDevice(), mCommandPool, nullptr);
		mCommandPool = VK_NULL_HANDLE;
		mCommandBuffers.clear();
		mManager = nullptr;
		mIsReady = false;
	}

	MxVulkanCommand::~MxVulkanCommand()
	{
		destroy();
	}
}
