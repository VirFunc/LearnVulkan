#pragma once
#ifndef _MX_VULKAN_UTILS_H_
#define _MX_VULKAN_UTILS_H_

#include<vulkan/vulkan.h>

#include<stdexcept>
#include<string>



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

	std::string mxErrorString(const VkResult res);


}
#endif // !_MX_VULKAN_UTILS_H_

//check the result returned by vkxxx
#define MX_VK_CHECK_RESULT(r)										\
{																	\
	VkResult result=(r);											\
	if(result != VK_SUCCESS)										\
	{																\
		throw std::runtime_error("Error : "+Mixel::mxErrorString(result)+" at "+__FILE__+" : "+std::to_string(__LINE__));  				\
	}																\
}