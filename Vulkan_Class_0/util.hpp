#pragma once
#include<vulkan/vulkan.h>

struct SwapchainImage
{
	VkImage image;
	VkImageView view;
};

struct Buffer
{
	VkBuffer buffer;
	VkDeviceMemory memory;
};

struct Image
{
	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;
};