#include "MxVulkanDebug.h"

namespace Mixel
{
	VKAPI_ATTR VkBool32 VKAPI_CALL MxVulkanDebug::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData, void * pUserData)
	{
		std::string msg = "[ Validation layer ] : [ ";

		switch (messageType)
		{
		case TYPE_GENERAL:
			msg += "General : ";
			break;
		case TYPE_VALIDATION:
			msg += "Validation : ";
			break;
		case TYPE_PERFORMANCE:
			msg += "Performance : ";
			break;
		}

		switch (messageSeverity)
		{
		case SEVERITY_WARNING:
			msg += "Warning";
			break;
		case SEVERITY_ERROR:
			msg += "Error";
			break;
		case SEVERITY_VERBOSE:
			msg += "Verbose";
			break;
		case SEVERITY_INFO:
			msg += "Infomation";
			break;
		}
		
		msg = msg + " ]\n\t" + pCallbackData->pMessage;
		std::cerr << std::endl << msg << std::endl;
		return VK_FALSE;
	}

	MxVulkanDebug::MxVulkanDebug() :mIsReady(false), mManager(nullptr),
		mCreateDebugUtilsMessenger(VK_NULL_HANDLE), mDestroyDebugUtilsMessenger(VK_NULL_HANDLE)
	{
	}

	bool MxVulkanDebug::setup(const MxVulkanManager* manager)
	{
		if (mIsReady)
			destroy();
		mManager = manager;
		auto creater = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mManager->getInstance(), "vkCreateDebugUtilsMessengerEXT");
		auto destroyer = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mManager->getInstance(), "vkDestroyDebugUtilsMessengerEXT");
		if (!creater || !destroyer)
		{ 
			mIsReady = false;
			return false;
		}
		else
		{
			mIsReady = true;
			mCreateDebugUtilsMessenger = creater;
			mDestroyDebugUtilsMessenger = destroyer;
			return true;
		}
	}

	MxVulkanDebug::~MxVulkanDebug()
	{
		destroy();
	}

	bool MxVulkanDebug::setDebugCallback(Severity severity, Type type, PFN_vkDebugUtilsMessengerCallbackEXT callback, void * userData)
	{
		if (!mIsReady)
			return false;

		VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = severity;
		createInfo.messageType = type;
		createInfo.pfnUserCallback = callback;

		if (mCreateDebugUtilsMessenger(mManager->getInstance(), &createInfo, nullptr, &messenger) != VK_SUCCESS)
		{
			throw std::runtime_error("Error : Failed to setup debug callback!");
		}
		mMessengers.push_back(messenger);
		return true;
	}

	bool MxVulkanDebug::setDefaultCallback(Severity severity, Type type)
	{
		if (!mIsReady)
			return false;
		return setDebugCallback(severity, type, debugCallback, nullptr);
	}

	void MxVulkanDebug::destroy()
	{
		if (!mIsReady)
			return;
		for (const auto& messenger : mMessengers)
		{
			mDestroyDebugUtilsMessenger(mManager->getInstance(), messenger, nullptr);
		}
		mMessengers.clear();
		mManager = nullptr;
		mIsReady = false;
	}

}