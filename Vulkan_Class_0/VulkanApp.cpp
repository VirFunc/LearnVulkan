#include "VulkanApp.h"

VulkanRender::VulkanRender() :window(nullptr), paused(false)
{
}


VulkanRender::~VulkanRender()
{
}

void VulkanRender::createInstance()
{
	if (validationLayers.enabled && !validationLayers.checkSupport())
	{
		throw std::runtime_error("Error : Validation layers required, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Demo";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "Mixel";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	if (validationLayers.enabled)
	{
		createInfo.ppEnabledExtensionNames = validationLayers.layers.data();
	}
}
