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
		VkFormat format;
		VkExtent2D extent;

		static VkImage createImage2D(const MxVulkanManager* manager,
									 const VkExtent2D extent,
									 const VkFormat format,
									 const VkImageUsageFlags usage,
									 const VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
									 const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
									 const VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
									 const VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
									 const uint32_t mipLevels = 1, const uint32_t arrayLayers = 1);

		static VkImageView createImageView2D(const MxVulkanManager* manager,
											 const VkImage image,
											 const VkFormat format,
											 const VkImageAspectFlags aspectFlags,
											 const uint32_t mipLevel = 0,
											 const uint32_t levelCount = 1,
											 const uint32_t layer = 0,
											 const uint32_t layerCount = 1);

		static VkDeviceMemory allocateImageMemory(const MxVulkanManager* manager, const VkImage image,
												  const VkMemoryPropertyFlags properties);

		static MxVulkanImage* createDepthStencil(const MxVulkanManager* manager,
												 const VkFormat format, const VkExtent2D& extent, const VkSampleCountFlagBits sampleCount);
	};
}
#endif // !_MX_VULKAN_IMAGE_H_