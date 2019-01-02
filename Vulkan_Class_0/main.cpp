#include"MxVulkanDebug.h"
#include"MxVulkanSwapchain.h"
#include"MxVulkanRenderPass.h"
#include"MxVulkanManager.h"
#include"MxVulkanDescriptor.h"
#include"MxWindow.h"

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		throw std::runtime_error("Error : Failed to initialize SDL2!");
	}


	Mixel::MxWindow* window = new Mixel::MxWindow;
	Mixel::MxVulkanManager* manager = new Mixel::MxVulkanManager;
	Mixel::MxVulkanDebug* debug = new Mixel::MxVulkanDebug;
	Mixel::MxVulkanSwapchain* swapchain = new Mixel::MxVulkanSwapchain;
	Mixel::MxVulkanRenderPass* renderPass = new Mixel::MxVulkanRenderPass;
	Mixel::MxVulkanDescriptorPool* descriptorPool = new Mixel::MxVulkanDescriptorPool;
	Mixel::MxVulkanDescriptorSetLayout* descriptorSetLayout = new Mixel::MxVulkanDescriptorSetLayout;
	

	window->create("Demo", 600, 480);

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
	info->window = window->getWindow();

	int width, height;
	SDL_Vulkan_GetDrawableSize(window->getWindow(), &width, &height);

	try
	{
		manager->initialize(*info);
		debug->setup(manager);
		swapchain->setup(manager);
		renderPass->setup(manager);
		descriptorSetLayout->setup(manager);

		debug->setDefaultCallback(Mixel::MxVulkanDebug::SEVERITY_ALL, Mixel::MxVulkanDebug::TYPE_ALL);

		swapchain->createSwapchain({ {VK_FORMAT_B8G8R8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } },
								   VK_PRESENT_MODE_MAILBOX_KHR, { uint32_t(width),uint32_t(height) });

		auto colorAttachIndex = renderPass->addColorAttach(swapchain->getCurrFormat().format, VK_SAMPLE_COUNT_1_BIT,
														   VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
														   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		auto colorRefIndex = renderPass->addColorAttachRef(colorAttachIndex);
		auto subpassIndex = renderPass->addSubpass();
		renderPass->addSubpassColorRef(subpassIndex, { colorRefIndex });
		renderPass->addDependency(VK_SUBPASS_EXTERNAL, subpassIndex,
								  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
								  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
								  0,
								  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
								  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
		renderPass->createRenderPass();

		descriptorSetLayout->addBindings(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
		descriptorSetLayout->addBindings(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
		descriptorSetLayout->createDescriptorSetLayout();


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

	if (renderPass)
		delete renderPass;
	if (swapchain)
		delete swapchain;
	if (debug)
		delete debug;
	if (manager)
		delete manager;
	return 0;
}
