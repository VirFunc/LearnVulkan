#include "MxVulkanDebug.h"

namespace Mixel
{
	bool MxVulkanDebug::setup(const VkInstance & instance)
	{
		auto creater = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		auto destroyer = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (!creater || !destroyer)
			return false;
		else
			return true;
	}

	MxVulkanDebug::~MxVulkanDebug()
	{
		for (const auto& messenger : messengers)
		{
			destroyDebugUtilsMessenger(instance, messenger, nullptr);
		}
	}

	VkResult MxVulkanDebug::pushDebugCallback(VkDebugUtilsMessageSeverityFlagsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, PFN_vkDebugUtilsMessengerCallbackEXT callback, void * userData)
	{
		messengers.push_back(VK_NULL_HANDLE);
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = severity;
		createInfo.messageType = type;
		createInfo.pfnUserCallback = callback;
		
		if (createDebugUtilsMessenger(instance, &createInfo, nullptr, &(*messengers.end())) != VK_SUCCESS)
		{
			throw std::runtime_error("Error : Failed to setup debug callback!");
		}
	}

}