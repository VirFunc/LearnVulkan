#pragma once
#ifndef _MX_VULKAN_UTILS_H_
#define _MX_VULKAN_UTILS_H_

#include<vulkan/vulkan.h>

#include<stdexcept>

namespace Mixel
{
	

	struct MxVulkanQueueFamilyIndices
	{
		uint32_t graphics;
		uint32_t present;
		uint32_t compute;
	};

	struct MxVulkanQueue
	{
		VkQueue graphics;
		VkQueue present;
		VkQueue compute;
	};
}
#endif // !_MX_VULKAN_UTILS_H_
