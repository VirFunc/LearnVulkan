#pragma once
#ifndef _MX_VULKAN_IMAGE_H_
#define _MX_VULKAN_IMAGE_H_

#include"MxVulkanManager.h"

namespace Mixel
{
	struct MxVulkanImage
	{
		VkImage image;
		VkImageView view;
		VkDeviceMemory memory;

		static VkImage createImage2D(const MxVulkanManager* manager,
									 const VkExtent2D extent,
									 const VkFormat format,
									 const VkImageUsageFlags usage,
									 const VkImageTiling tiling,
									 const VkImageLayout initialLayout,
									 const VkSharingMode sharingMode,
									 const uint32_t mipLevels = 1,
									 const uint32_t arrayLayers = 1, const VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

		static VkImageView createImageView2D(const MxVulkanManager* manager,
											 const VkImage image,
											 const VkFormat format,
											 const VkImageAspectFlags aspectFlags,
											 const uint32_t mipLevel,
											 const uint32_t levelCount,
											 const uint32_t layer,
											 const uint32_t layerCount);

		static VkDeviceMemory allocateImageMemory(const MxVulkanManager* manager, const VkImage image,
												  const VkMemoryPropertyFlags properties);
	};
}
#endif // !_MX_VULKAN_IMAGE_H_