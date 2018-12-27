#include"MxVulkanDebug.h"
#include"MxVulkanSwapchain.h"
#include"MxVulkanManager.h"

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

int main()
{
	SDL_Window* window;
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		throw std::runtime_error("Error : Failed to initialize SDL2!");
	}

	window = SDL_CreateWindow("Vulkan on SDL2 Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							  600, 480, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	Mixel::MxVulkanManager* manager = new Mixel::MxVulkanManager;
	Mixel::MxVulkanDebug* debug = new Mixel::MxVulkanDebug;
	Mixel::MxVulkanSwapchain* swapchain = new Mixel::MxVulkanSwapchain;

	auto* info = manager->getEmptyInitInfo();
	info->instance.appInfo.appName = "Demo";
	info->instance.appInfo.appVersion = VK_MAKE_VERSION(0, 0, 5);
	info->instance.appInfo.engineName = "Mixel";
	info->instance.appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 5);

	info->instance.validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");

	info->debugMode = true;
	info->present = true;
	info->device.physical.enabledFeatures = { false };
	info->device.physical.extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	info->device.physical.type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	info->device.physical.queueFlags = VK_QUEUE_GRAPHICS_BIT;
	info->device.physical.queuePrioriy.compute = 1.0f;
	info->device.physical.queuePrioriy.graphics = 1.0f;
	info->device.physical.queuePrioriy.present = 1.0f;
	info->window = window;

	int width, height;
	SDL_Vulkan_GetDrawableSize(window, &width, &height);

	try
	{
		manager->initialize(*info);
		debug->setup(manager);
		debug->setDefaultCallback(Mixel::MxVulkanDebug::SEVERITY_ALL, Mixel::MxVulkanDebug::TYPE_ALL);
		swapchain->setup(manager);
		swapchain->createSwapchain({ {VK_FORMAT_B8G8R8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } },
								   VK_PRESENT_MODE_MAILBOX_KHR, { uint32_t(width),uint32_t(height) });
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	delete info;
	bool quit = false;
	SDL_Event event;
	while (!quit)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				quit = true;
				break;
			}
		}
	}
	if (swapchain)
		delete swapchain;
	if (debug)
		delete debug;
	if (manager)
		delete manager;
	return 0;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData,
	void * pUserData)
{
	std::string msg = "[ Validation layer ] : [ ";
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		msg += "Warning";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		msg += "Error";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		msg += "Verbose";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		msg += "Infomation";
		break;
	}
	msg = msg + " ]\n\t" + pCallbackData->pMessage;
	std::cerr << std::endl << msg << std::endl;
	return VK_FALSE;
}
