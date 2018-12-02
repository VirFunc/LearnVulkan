#include"Demo.h"

Demo::Demo()
{
}

Demo::~Demo()
{
}

void Demo::initWindow()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		throw std::runtime_error("Error : Failed to initialize SDL2!");
	}

	window = SDL_CreateWindow("Vulkan on SDL2 Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							  WIDTH, HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	if (!window)
	{
		throw std::runtime_error("Error : Failed to create window!");
	}
}

void Demo::initVulkan()
{
	createInstance();
	setupDebugCallback();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createVertexBuffer();
	createIndexBuffer();
	createCommandBuffers();
	createSyncObjects();
}

void Demo::mainLoop()
{
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

			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					framebufferResized = true;
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		drawFrame();
	}
}

void Demo::cleanup()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Demo::drawFrame()
{
	vkWaitForFences(device, 1, &inFlightFences[currFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

	uint32_t imageIndex;
	//��ȡ��һ��image
	VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(),
											imageAvailableSemaphores[currFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Error : Failed to acquire next image!");

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	submitInfo.commandBufferCount = 1;

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currFrame] };
	submitInfo.pSignalSemaphores = signalSemaphores;
	submitInfo.signalSemaphoreCount = 1;

	vkResetFences(device, 1, &inFlightFences[currFrame]);
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currFrame]) != VK_SUCCESS)
		throw std::runtime_error("Error : Failed  to submit draw command buffer!");

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.waitSemaphoreCount = 1;
	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.pSwapchains = swapChains;
	presentInfo.swapchainCount = 1;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present swap chain image!");

	currFrame = (currFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

//����ʵ��
void Demo::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Error : Validation layers required, but not available!");
	}

	//vulkanӦ����Ϣ
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan on SDL2 Demo";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "AE";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	//ʵ��������Ϣ
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	//��֤��
	if (enableValidationLayers)
	{
		createInfo.ppEnabledLayerNames = validationLayers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(validationLayers.size());
	}
	else
	{
		createInfo.ppEnabledLayerNames = nullptr;
		createInfo.enabledLayerCount = 0;
	}

	//��Ҫ����չ
	auto extensionRequired = std::move(getRequiredExtensions());
	createInfo.ppEnabledExtensionNames = extensionRequired.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionRequired.size());

	//��֧�ֵ���չ
	uint32_t extentionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extentionCount, nullptr);
	std::vector<VkExtensionProperties> extensionSupported(extentionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extentionCount, extensionSupported.data());

	std::cout << "[ Extention ] supported :" << std::endl;
	for (auto& a : extensionSupported)
	{
		std::cout << "\t[ " << a.extensionName << " ]" << std::endl;
	}

	std::cout << std::endl << "[ Extention ] required:" << std::endl;
	for (auto& a : extensionRequired)
	{
		std::cout << "\t[ " << a << " ]" << std::endl;
	}

	//����ʵ��
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create Vulkan instance");
	}
}

//����debug�ص�
void Demo::setupDebugCallback()
{
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	//Ҫ���ܵ���Ϣ�ĵȼ�
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	//Ҫ���ܵ���Ϣ������
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback; //�ص�����
	createInfo.pUserData = nullptr; //�û�����

	if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to setup debug callback!");
	}
}

//����surface
void Demo::createSurface()
{
	if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE)
	{
		throw std::runtime_error("Error : Failed to create window surface!");
	}
}

//ѡ����ʵ������豸
void Demo::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	//ö�������豸
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount < 1)
		throw std::runtime_error("Error : Failed to find device with Vulkan support!");
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			//��ȡ���ʵ��豸�Ķ��д�
			queueFamilyIndices = findQueueFamilies(physicalDevice);
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Error : Failed to find suitable device!");
}

//�����߼��豸
void Demo::createLogicalDevice()
{
	//��Ҫʹ�õĶ��д�
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies =
	{
		queueFamilyIndices.graphicsFamily.value(),
		queueFamilyIndices.presentFamily.value()
	};
	float queuePrioriy = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePrioriy;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	//ѡ��Ҫʹ�õ������豸����
	VkPhysicalDeviceFeatures deviceFeatures = {};

	//�����߼��豸
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());

	if (enableValidationLayers)
	{
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	}
	else
	{
		deviceCreateInfo.ppEnabledLayerNames = nullptr;
		deviceCreateInfo.enabledLayerCount = 0;
	}

	//�����豸
	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create logical device!");
	}

	//��ȡ�豸����
	vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
}

//����������
void Demo::createSwapChain()
{
	//��ȡ����������������Ϣ
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	//������surface��ʽ
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	//����������ģʽ
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	//������
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	//��������image��������Ϊ�˼��������壬�˴�������һ��
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (imageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	//����������
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.presentMode = presentMode;
	createInfo.minImageCount = imageCount;
	//��ʽ
	createInfo.imageFormat = surfaceFormat.format;
	//ɫ�ʿռ�
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	//ͼ���С
	createInfo.imageExtent = extent;
	//ÿһ��image�����Ĳ���
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //image����;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),indices.presentFamily.value() };
	if (indices.graphicsFamily.value() != indices.presentFamily.value())
	{//���graphics����present�ز�ͬ
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; //����
		//�����ͬ��ͨ��queueFamilyIndices�����ֲ�ͬ�Ķ��д�
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
		createInfo.queueFamilyIndexCount = 2;
	}
	else
	{//���graphics����present����ͬ
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //��ռ
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	//�Խ������е�image������Ԥ�任
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	//ȷ�����봰��ϵͳ�е��������ڻ�����Ƿ�ʹ��alphaͨ��
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	//�����ڱ����������ڵ�ʱ���Ƿ�ü������ڵ��Ĳ���
	createInfo.clipped = true;
	//�������������ı�ʱ��������һ��������
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	//����������
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create swap chain!");
	}

	//��ȡ�������е�image
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	//���潻�����ĸ�ʽ�ʹ�С
	swapChainFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

//����image view
void Demo::createImageViews()
{
	size_t size = swapChainImages.size();
	swapChainImageViews.resize(size);

	for (size_t i = 0; i < size; ++i)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainFormat; //image��ʽ
		//ͨ��ӳ��
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		//ͼ�����;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0; //����mipmap����
		createInfo.subresourceRange.levelCount = 1; //mipmap����
		createInfo.subresourceRange.baseArrayLayer = 0; //����ͼ�����
		createInfo.subresourceRange.layerCount = 1; //�ܲ���

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create image view!");
	}
}

//����render pass
void Demo::createRenderPass()
{
	//��ɫ��������
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainFormat; //image view�ĸ�ʽ
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //��subpass��ʼʱ���������ݵĲ���
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //��subpass����ʱ�Ƿ�����ݴ洢
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //��ģ�建��Ĳ���
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //renderpass��ʼʱimage�Ĳ���
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //renderpass����ʱimage�Ĳ���

	//��������
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0; //attachment������
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //����������ʱ�Ĳ���(layout)

	//��ͨ������
	//ÿ����ͨ��������һ������ ����(attachment)
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //��subpass�󶨵�graphics�ܵ�
	subpass.pColorAttachments = &colorAttachmentRef; //��ɫ��������
	subpass.colorAttachmentCount = 1;

	//subpass�ᴦ��image��layout��ת��
	//���ת���� SubpassDependency ������
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; //��ʽsubpass
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	//����render pass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = &colorAttachment; //renderpass���еĸ���
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pSubpasses = &subpass; //renderpass��������subpass
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pDependencies = &dependency;
	renderPassInfo.dependencyCount = 1;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create render pass!");
	}
}

void Demo::createGraphicsPipeline()
{
	//������ɫ��
	auto vertShaderCode = readFile("Shader\\vert.spv");
	auto fragShaderCode = readFile("Shader\\frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	//shader stage
	//������ ��ɫ����ʹ�õĽ׶�/��ɫ������ں��� ����Ϣ
	VkPipelineShaderStageCreateInfo vertShaderStage = {};
	vertShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStage.module = vertShaderModule;
	vertShaderStage.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStage = {};
	fragShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStage.module = fragShaderModule;
	fragShaderStage.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStageInfos[] = { vertShaderStage,fragShaderStage };

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescription = Vertex::getAttributeDescription();

	//��������˵��
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());

	//����װ��
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	//������Ķ����������װ��
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//�Ƿ����� ͼԪ����
	inputAssembly.primitiveRestartEnable = false;

	//�ӿ�
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)WIDTH;
	viewport.height = (float)HEIGHT;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	//�ü�
	VkRect2D scissor = { };
	scissor.extent = swapChainExtent;
	scissor.offset = { 0,0 };

	//������ �ӿ�/�ü� ˵��
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pViewports = &viewport;
	viewportState.viewportCount = 1;
	viewportState.pScissors = &scissor;
	viewportState.scissorCount = 1;

	//��դ��
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = false; //��Ϊ�棬��������Զƽ���Ƭ�λᱻ�ض϶����Ƕ���
	rasterizer.rasterizerDiscardEnable = false; //��Ϊ�棬����Ƭ�ζ��ᱻ����
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //��������ģʽ
	rasterizer.lineWidth = 1.0f; //�߿�
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //���޳�ģʽ
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; //����Ϊ����
	//ʹ��դ��ͨ�� ����һ������/����ͼԪ��б�� ���ı������ֵ
	rasterizer.depthBiasEnable = false;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;

	//���ز���
	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = false;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = false;
	multisampleState.alphaToOneEnable = false;

	//��Ȼ���

	//ģ�建��

	//���
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.blendEnable = true; //�Ƿ���blend
	// ��Щ��ɿ��Ա�д��
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	//��Ϸ����еĸ�������
	//finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	//finalColor.a   = (srcAlphaBlendFactor * newColor.a)   <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = false;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachment;
	colorBlendState.blendConstants[0] = 0.0f;
	colorBlendState.blendConstants[1] = 0.0f;
	colorBlendState.blendConstants[2] = 0.0f;
	colorBlendState.blendConstants[3] = 0.0f;

	//��̬״̬

	//���߲���
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("Error : Failed to create pipeline layout!");

	//����ͼ����Ⱦ����
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pStages = shaderStageInfos; //shader stage
	pipelineInfo.stageCount = 2;
	pipelineInfo.pVertexInputState = &vertexInputInfo;  //��������˵��
	pipelineInfo.pInputAssemblyState = &inputAssembly;  //����װ��˵��
	pipelineInfo.pViewportState = &viewportState;	    //�ӿ�˵��
	pipelineInfo.pRasterizationState = &rasterizer;     //��դ��˵��
	pipelineInfo.pMultisampleState = &multisampleState; //���ز���˵��
	pipelineInfo.pDepthStencilState = nullptr;			//���/ģ��˵��
	pipelineInfo.pColorBlendState = &colorBlendState;	//��ɫ���˵��
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = pipelineLayout;				//���߲���
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;	//�������ߣ�vulkan�������Ѿ����ڵĹ����������µĹ��ߣ�
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		throw std::runtime_error("Error : Failed to create graphics pipeline!");
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

//����framebuffer
void Demo::createFramebuffers()
{
	//Ϊswapchain�е�ÿһ��image����framebuffer
	swapChainFramebuffers.resize(swapChainImageViews.size());

	size_t i = 0;
	for (const auto& imageViews : swapChainImageViews)
	{
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass; //ȷ��framebuffer�����ĸ�renderpass����
		framebufferInfo.pAttachments = &imageViews; //Ϊframebufferָ��imageview
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1; //image������layer����

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i++]) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create framebuffer!");
	}
}

//���������
void Demo::createCommandPool()
{
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	commandPoolInfo.flags = 0;
	if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw std::runtime_error("Error : Failed to create command pool!");
}

void Demo::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(decltype(vertices)::value_type)*vertices.size();
	//��ʱ����
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				 stagingBuffer, stagingBufferMemory);
	//�ڴ�ӳ��
	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
	//���ƻ�����
	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	//������ʱ����
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Demo::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(decltype(indices)::value_type)*indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				 stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Demo::createCommandBuffers()
{
	//Command Buffer
	//	|->Render Pass Begin
	//		|->Bind Graphics pipeline
	//		|->State Management
	//		|->Bind Vertex Buffer
	//		|->Update Vertex Buffer(output)
	//		|->Bind Description Set
	//		|->Draw
	//		|->Execute Command
	//	|=>Render Pass End

	commandBuffers.resize(swapChainFramebuffers.size());

	//Ϊÿһ��swap chain framebuffer ���������
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //����ֱ���������ִ�У������ܱ�����command buffer����
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("Error : Failed to allocate command buffers!");

	size_t size = commandBuffers.size();
	for (size_t i = 0; i < size; ++i)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //���������ڵȴ�ִ��ʱ�ٴ��ύ
		beginInfo.pInheritanceInfo = nullptr;

		//��ʼ��¼command
		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to begin recording command buffer!");

		VkRenderPassBeginInfo renderInfo = {};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderInfo.renderPass = renderPass;
		renderInfo.framebuffer = swapChainFramebuffers[i];
		renderInfo.renderArea.offset = { 0,0 };
		renderInfo.renderArea.extent = swapChainExtent;
		VkClearValue clearColor = { 0.0f,0.0f,0.0f,1.0f };
		renderInfo.pClearValues = &clearColor;
		renderInfo.clearValueCount = 1;

		//��ʼһ��renderpass
		vkCmdBeginRenderPass(commandBuffers[i], &renderInfo, VK_SUBPASS_CONTENTS_INLINE);
		//Ϊcommand buffer��pipeline
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		//�󶨶��㻺��
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		//����
		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		//����renderpass
		vkCmdEndRenderPass(commandBuffers[i]);
		//����һ��command buffer
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to record command buffer!");
	}
}

void Demo::createSyncObjects()
{
	//VkSemaphore  ����ͬ��һ����������
	//VkFence      ����ͬ��������������Ⱦ
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo  semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS)
			throw std::runtime_error("Error ��Failed to create semaphore!");
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create semaphore!");
		if (vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create fence!");
	}
}

//���´���������
void Demo::recreateSwapChain()
{
	//�ȴ��豸ִ�����
	vkDeviceWaitIdle(device);
	cleanupSwapChain();

	createSwapChain();
	createImageViews(); //image view����swapchain�е�image
	createRenderPass(); //render pass����swapchain��format
	createGraphicsPipeline(); //graphics pipeline��������viewport & scissor
	createFramebuffers();
	createCommandBuffers();
}

//��������
void Demo::cleanupSwapChain()
{
	for (auto& framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (auto& imageView : swapChainImageViews)
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
}

//�����֤��֧��
bool Demo::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayer(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayer.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayer)
		{
			if (strcmp(layerProperties.layerName, layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
			return false;
	}
	return true;
}

//��������豸�Ƿ�֧����չ
bool Demo::checkDeviceExtionSupport(VkPhysicalDevice & physicalDevice)
{
	//ö���豸��չ����
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	bool found = false;
	for (auto& requriedExt : deviceExtensions)
	{
		found = false;
		for (auto& availableExt : availableExtensions)
		{
			if (strcmp(requriedExt, availableExt.extensionName) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
			break;
	}

	return found;
}

SwapChainSupportDetails Demo::querySwapChainSupport(VkPhysicalDevice & device)
{//��ȡ������ϸ����Ϣ
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

//��������豸�Ƿ����Ҫ��
bool Demo::isDeviceSuitable(VkPhysicalDevice & physicalDevice)
{
	//��ȡ�����豸����
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

	//��������豸�����Ƿ����Ҫ��
	bool devicePorpertiesSuit = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
		&& deviceFeatures.geometryShader;

	//��������豸���д��Ƿ����Ҫ��
	bool queueFamilySuit = findQueueFamilies(physicalDevice).isComplete();

	//��������豸��չ�Ƿ�֧��
	bool deviceExtensionSuit = checkDeviceExtionSupport(physicalDevice);

	//��⽻�����Ƿ�֧��
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
	bool swapChainSuit = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

	if (queueFamilySuit && deviceExtensionSuit && swapChainSuit)
	{
		std::cout << std::endl << "[ Physical device ] :" << std::endl
			<< "\tDevice name : " << deviceProperties.deviceName << std::endl;
		return true;
	}
	else
		return false;
}

//Ѱ�Һ��ʵĶ��д�
QueueFamilyIndices Demo::findQueueFamilies(VkPhysicalDevice & physicalDevice)
{
	//��ȡ�����豸���д�����
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	uint32_t i = 0;
	for (auto& queueFamily : queueFamilies)
	{
		//�����Ƿ�֧��graphics
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		//�����Ƿ�֧��present
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport)
			indices.presentFamily = i;

		if (indices.isComplete())
			break;
		++i;
	}

	return indices;
}

VkSurfaceFormatKHR Demo::chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	for (const auto& format : availableFormats)
	{
		if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	}

	//fallback
	return availableFormats[0];
}

VkPresentModeKHR Demo::chooseSwapPresentMode(std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& presentMode : availablePresentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Demo::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		SDL_Vulkan_GetDrawableSize(window, &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		actualExtent.width = std::max(capabilities.minImageExtent.width,
									  std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height,
									   std::min(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	}
}

// ��ȡ��Ҫ�ĳ�����Ҫ����չ�б�
std::vector<const char*> Demo::getRequiredExtensions()
{
	uint32_t sdlExtentionCount = 0;
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtentionCount, nullptr);
	std::vector<const char*> extensions(sdlExtentionCount);
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtentionCount, extensions.data());
	if (enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Demo::debugCallback(
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
	}
	msg = msg + " ]\n\t" + pCallbackData->pMessage;
	std::cerr << std::endl << msg << std::endl;
	return VK_FALSE;
}

std::vector<char> Demo::readFile(const std::string & path)
{
	std::ifstream inFile(path, std::ios_base::ate | std::ios_base::binary);
	if (!inFile.is_open())
		throw std::runtime_error("Error : Failed to open file");
	size_t fileSize = static_cast<size_t>(inFile.tellg());
	std::vector<char> buffer(fileSize);
	inFile.seekg(std::ios_base::beg);
	inFile.read(buffer.data(), fileSize);
	inFile.close();
	return buffer;
}

VkResult createDebugUtilsMessengerEXT(VkInstance instance,
									  const VkDebugUtilsMessengerCreateInfoEXT * pCreateInfo,
									  const VkAllocationCallbacks * pAllocator,
									  VkDebugUtilsMessengerEXT * pCallback)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pCallback);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroyDebugUtilsMessengerEXT(VkInstance instance,
								   VkDebugUtilsMessengerEXT callback,
								   const VkAllocationCallbacks * pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, callback, pAllocator);
}

VkShaderModule Demo::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Error : Failed to create shader module!");
	return shaderModule;
}

uint32_t Demo::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("Error : Failed to find suitable memory type!");
}

void Demo::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer & buffer, VkDeviceMemory & bufferMemory)
{
	//��������
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create buffer!");
	}
	//��ȡ�ڴ�����
	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
	//�����ڴ�
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to allocate memory!");
	}
	//���ڴ浽������
	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void Demo::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	//������ʱcommand buffer
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	//��¼command
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //������ֻ���ύһ��,��������

	//��ʼ��¼
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	//���ƻ���
	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	//������¼
	vkEndCommandBuffer(commandBuffer);

	//�ύcommand
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.commandBufferCount = 1;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue); //�ȴ��ύ���

	//�ͷ���ʱcommand buffer
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}