#include "MxVulkanImage.h"

namespace Mixel
{
	VkImage MxVulkanImage::createImage2D(const MxVulkanManager* manager, const VkExtent2D extent, const VkFormat format, const VkImageUsageFlags usage, const VkImageTiling tiling, const VkImageLayout initialLayout, const VkSharingMode sharingMode, const uint32_t mipLevels, const uint32_t arrayLayers, const VkSampleCountFlagBits sampleCount)
	{
		VkImageCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.extent.width = extent.width;
		createInfo.extent.height = extent.height;
		createInfo.extent.depth = 1;
		createInfo.mipLevels = mipLevels;
		createInfo.arrayLayers = arrayLayers;
		createInfo.format = format;
		createInfo.tiling = tiling;
		createInfo.initialLayout = initialLayout;
		createInfo.usage = usage;
		createInfo.sharingMode = sharingMode;
		createInfo.samples = sampleCount;

		VkImage tempImage;
		if (vkCreateImage(manager->getDevice(), &createInfo, nullptr, &tempImage) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create image");
		return tempImage;
	}

	VkImageView MxVulkanImage::createImageView2D(const MxVulkanManager* manager, const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags, const uint32_t mipLevel, const uint32_t levelCount, const uint32_t layer, const uint32_t layerCount)
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = mipLevel; //the first mipmap level accessible to the view
		viewInfo.subresourceRange.levelCount = levelCount; //the number of mipmap levels (starting from baseMipLevel) accessible to the view
		viewInfo.subresourceRange.baseArrayLayer = layer;
		viewInfo.subresourceRange.layerCount = layerCount;

		VkImageView imageView;
		if (vkCreateImageView(manager->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Error : Failed to create image view!");
		}
		return imageView;
	}
	VkDeviceMemory MxVulkanImage::allocateImageMemory(const MxVulkanManager* manager, const VkImage image, const VkMemoryPropertyFlags properties)
	{
		VkMemoryRequirements memRequirements = {};
		vkGetImageMemoryRequirements(manager->getDevice(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = manager->getMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

		VkDeviceMemory tempMemory;
		if (vkAllocateMemory(manager->getDevice(), &allocInfo, nullptr, &tempMemory) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to allocate memory");
		return tempMemory;
	}
}
