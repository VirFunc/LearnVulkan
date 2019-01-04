#pragma once
#ifndef _MX_VULKAN_BUFFER_H_
#define _MX_VULKAN_BUFFER_H_

#include<cassert>

namespace Mixel
{
	class  MxVulkanManager;

	struct MxVulkanBuffer
	{
		const MxVulkanManager* manager;
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo descriptor;
		VkDeviceSize size;
		VkDeviceSize alignment;
		void* mapped;

		VkBufferUsageFlags usages;
		VkMemoryPropertyFlags memoryProperty;

		VkResult map(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);
		void unmap();
		VkResult bind(const VkDeviceSize offset = 0);
		void setupDescriptor(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);
		void copyTo(void* data, const VkDeviceSize size);
		VkResult flush(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);
		VkResult invalidate(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);
		void destory();
		~MxVulkanBuffer() { destory(); };
	};

	VkResult MxVulkanBuffer::map(const VkDeviceSize size, const VkDeviceSize offset)
	{
		return vkMapMemory(manager->getDevice(), memory, offset, size, 0, &mapped);
	}

	void MxVulkanBuffer::unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(manager->getDevice(), memory);
			mapped = nullptr;
		}
	}

	VkResult MxVulkanBuffer::bind(const VkDeviceSize offset)
	{
		return vkBindBufferMemory(manager->getDevice(), buffer, memory, offset);
	}

	void MxVulkanBuffer::setupDescriptor(const VkDeviceSize size, const VkDeviceSize offset)
	{
		descriptor.offset = offset;
		descriptor.buffer = buffer;
		descriptor.range = size;
	}

	void MxVulkanBuffer::copyTo(void * data, const VkDeviceSize size)
	{
		assert(data);
		memcpy(mapped, data, size);
	}

	VkResult MxVulkanBuffer::flush(const VkDeviceSize size, const VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(manager->getDevice(), 1, &mappedRange);
	}

	inline VkResult MxVulkanBuffer::invalidate(const VkDeviceSize size, const VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(manager->getDevice(), 1, &mappedRange);
	}

	void MxVulkanBuffer::destory()
	{
		if (buffer)
		{
			vkDestroyBuffer(manager->getDevice(), buffer, nullptr);
			buffer = VK_NULL_HANDLE;
		}
		if (memory)
		{
			vkFreeMemory(manager->getDevice(), memory, nullptr);
			memory = VK_NULL_HANDLE;
		}
	}


}
#endif // !_MX_VULKAN_BUFFER_

