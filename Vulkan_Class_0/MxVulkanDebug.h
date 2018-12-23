#pragma once
#ifndef _MX_VULKAN_DEBUG_H_
#define _MX_VULKAN_DEBUG_H_

#include<vulkan/vulkan.h>

#include<vector>

namespace Mixel
{
	struct MxVulkanDebug
	{
		VkInstance instance;
		std::vector<VkDebugUtilsMessengerEXT> messengers;
		PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessenger;
		PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessenger;

		MxVulkanDebug() = default;
		bool setup(const VkInstance& instance);
		~MxVulkanDebug();
		VkResult pushDebugCallback(VkDebugUtilsMessageSeverityFlagsEXT severity,
							   VkDebugUtilsMessageTypeFlagsEXT type,
							   PFN_vkDebugUtilsMessengerCallbackEXT callback,
							   void* userData);
	};

}

#endif