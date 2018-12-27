#pragma once
#ifndef _MX_VULKAN_UTILS_H_
#define _MX_VULKAN_UTILS_H_

#include<vulkan/vulkan.h>

#include<stdexcept>

namespace Mixel
{
	struct MxVulkanImage
	{
		VkImage image;
		VkImageView view;
		VkDeviceMemory memory;

		static VkImageView createImageView2D(VkDevice device, VkImage image, VkFormat format, 
											 VkImageAspectFlags aspectFlags,
											 uint32_t mipLevel, uint32_t levelCount,
											 uint32_t layer, uint32_t layerCount);
	};

	struct MxVulkanQueueFamilyIndices
	{
		uint32_t graphics;
		uint32_t present;
		uint32_t compute;
	};
}
#endif // !_MX_VULKAN_UTILS_H_
