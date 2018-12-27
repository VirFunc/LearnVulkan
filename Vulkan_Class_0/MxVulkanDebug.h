#pragma once
#ifndef _MX_VULKAN_DEBUG_H_
#define _MX_VULKAN_DEBUG_H_
#define MX_DEBUG _DEBUG

#include<vulkan/vulkan.h>

#include"MxVulkanManager.h"

#include<iostream>
#include<string>
#include<vector>

namespace Mixel
{
	class MxVulkanDebug
	{
	private:
		bool mIsReady;

		const MxVulkanManager* mManager;
		std::vector<VkDebugUtilsMessengerEXT> mMessengers;
		PFN_vkCreateDebugUtilsMessengerEXT mCreateDebugUtilsMessenger;
		PFN_vkDestroyDebugUtilsMessengerEXT mDestroyDebugUtilsMessenger;

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
															VkDebugUtilsMessageTypeFlagsEXT messageType,
															const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
															void* pUserData);
	public:
		enum Severity
		{
			SEVERITY_VERBOSE = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
			SEVERITY_INFO = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
			SEVERITY_WARNING = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
			SEVERITY_ERROR = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			SEVERITY_ALL = SEVERITY_VERBOSE | SEVERITY_INFO | SEVERITY_WARNING | SEVERITY_ERROR
		};

		enum Type
		{
			TYPE_GENERAL = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
			TYPE_VALIDATION = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
			TYPE_PERFORMANCE = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			TYPE_ALL = TYPE_GENERAL | TYPE_VALIDATION | TYPE_PERFORMANCE
		};

		MxVulkanDebug();
		bool setup(const MxVulkanManager* manager);
		bool setDebugCallback(Severity severity,
							  Type type,
							  PFN_vkDebugUtilsMessengerCallbackEXT callback,
							  void* userData);
		bool setDefaultCallback(Severity severity, Type type);
		void destroy();
		~MxVulkanDebug();
	};

}

#endif