#include "MxVulkanUtils.h"

VkImageView Mixel::MxVulkanImage::createImageView2D(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevel, uint32_t levelCount, uint32_t layer, uint32_t layerCount)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = mipLevel;
	viewInfo.subresourceRange.levelCount = levelCount;
	viewInfo.subresourceRange.baseArrayLayer = layer;
	viewInfo.subresourceRange.layerCount = layerCount;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create image view!");
	}
	return imageView;
}
