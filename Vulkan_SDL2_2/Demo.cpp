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
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
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
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
	vkDestroyCommandPool(device, commandPool, nullptr);
	cleanupSwapChain();

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);
	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);

	for (size_t i = 0; i < swapChainImages.size(); ++i)
	{
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemroy[i], nullptr);
	}

	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers)
		destroyDebugUtilsMessengerEXT(instance, callback, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Demo::drawFrame()
{
	vkWaitForFences(device, 1, &inFlightFences[currFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

	uint32_t imageIndex;
	//获取下一张image
	VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(),
											imageAvailableSemaphores[currFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to acquire next image!");
	}

	//更新unifom buffer
	updateUniformBuffer(imageIndex);

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

//更新uniform buffer
void Demo::updateUniformBuffer(uint32_t currImage)
{
	//第一次运行的时刻
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currTime = std::chrono::high_resolution_clock::now(); //这一帧的时刻
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currTime - startTime).count();

	UniformBufferObj ubo = {};
	ubo.model = glm::mat4(1.0f);
	ubo.model = glm::rotate(ubo.model, time*glm::radians(90.f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::mat4(1.0f);
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height,
								0.1f, 10.0f);
	//注意，glm原本为GL所设计，GL与Vulkan的坐标系系统，y轴方向恰好相反
	ubo.proj[1][1] *= -1;

	//复制ubo到缓冲区中
	void* data;
	vkMapMemory(device, uniformBuffersMemroy[currImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, uniformBuffersMemroy[currImage]);
}

//创建实例
void Demo::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Error : Validation layers required, but not available!");
	}

	//vulkan应用信息
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan on SDL2 Demo";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "AE";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	//实例创建信息
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	//验证层
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

	//需要的扩展
	auto extensionRequired = std::move(getRequiredExtensions());
	createInfo.ppEnabledExtensionNames = extensionRequired.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionRequired.size());

	//被支持的扩展
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

	//创建实例
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create Vulkan instance");
	}
}

//设置debug回调
void Demo::setupDebugCallback()
{
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	//要接受的消息的等级
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	//要接受的消息的类型
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback; //回调函数
	createInfo.pUserData = nullptr; //用户数据

	if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to setup debug callback!");
	}
}

//创建surface
void Demo::createSurface()
{
	if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE)
	{
		throw std::runtime_error("Error : Failed to create window surface!");
	}
}

//选择合适的物理设备
void Demo::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	//枚举物理设备
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
			//获取合适的设备的队列簇
			queueFamilyIndices = findQueueFamilies(physicalDevice);
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Error : Failed to find suitable device!");
}

//创建逻辑设备
void Demo::createLogicalDevice()
{
	//将要使用的队列簇
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

	//选择将要使用的物理设备功能
	VkPhysicalDeviceFeatures deviceFeatures = {};

	//创建逻辑设备
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

	//创建设备
	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create logical device!");
	}

	//获取设备队列
	vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
}

//创建交换链
void Demo::createSwapChain()
{
	//获取创建交换链所需信息
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	//交换链surface格式
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	//交换链呈现模式
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	//交换链
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	//交换链中image的数量（为了兼容三缓冲，此处数量加一）
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (imageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	//创建交换链
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.presentMode = presentMode;
	createInfo.minImageCount = imageCount;
	//格式
	createInfo.imageFormat = surfaceFormat.format;
	//色彩空间
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	//图像大小
	createInfo.imageExtent = extent;
	//每一个image包含的层数
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //image的用途

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),indices.presentFamily.value() };
	if (indices.graphicsFamily.value() != indices.presentFamily.value())
	{//如果graphics簇与present簇不同
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; //共有
		//如果不同，通过queueFamilyIndices来区分不同的队列簇
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
		createInfo.queueFamilyIndexCount = 2;
	}
	else
	{//如果graphics簇与present簇相同
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //独占
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	//对交换链中的image所做的预变换
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	//确定在与窗口系统中的其他窗口混合是是否使用alpha通道
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	//当窗口被其他窗口遮挡时，是否裁剪掉被遮挡的部分
	createInfo.clipped = true;
	//当交换链发生改变时，填入上一个交换链
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	//创建交换链
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create swap chain!");
	}

	//获取交换链中的image
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	//保存交换链的格式和大小
	swapChainFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

//创建image view
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
		createInfo.format = swapChainFormat; //image格式
		//通道映射
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		//图像的用途
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0; //基础mipmap级别
		createInfo.subresourceRange.levelCount = 1; //mipmap总数
		createInfo.subresourceRange.baseArrayLayer = 0; //基础图像层数
		createInfo.subresourceRange.layerCount = 1; //总层数

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create image view!");
	}
}

//创建render pass
void Demo::createRenderPass()
{
	//颜色附件描述
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainFormat; //image view的格式
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //在subpass开始时对现有数据的操作
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //在subpass结束时是否对数据存储
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //对模板缓冲的操作
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //renderpass开始时image的布局
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //renderpass结束时image的布局

	//附件引用
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0; //attachment的索引
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //附件被引用时的布局(layout)

	//子通道描述
	//每个子通道会引用一个或多个 附件(attachment)
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //该subpass绑定到graphics管道
	subpass.pColorAttachments = &colorAttachmentRef; //颜色附件引用
	subpass.colorAttachmentCount = 1;

	//subpass会处理image的layout的转换
	//这个转换由 SubpassDependency 所控制
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; //隐式subpass
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	//创建render pass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = &colorAttachment; //renderpass所有的附件
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pSubpasses = &subpass; //renderpass所包含的subpass
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pDependencies = &dependency;
	renderPassInfo.dependencyCount = 1;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create render pass!");
	}
}

//创建描述符布局
void Demo::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0; //对应shader中的binding
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //描述符类型
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //描述符被引用的shader stage
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pBindings = &uboLayoutBinding;
	layoutInfo.bindingCount = 1;

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create descriptor set layout!");
	}
}

//创建渲染管线
void Demo::createGraphicsPipeline()
{
	//加载着色器
	auto vertShaderCode = readFile("Shader\\vert.spv");
	auto fragShaderCode = readFile("Shader\\frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	//shader stage
	//包含该 着色器被使用的阶段/着色器的入口函数 等信息
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

	//顶点输入说明
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());

	//输入装配
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	//对输入的顶点数据如何装配
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//是否允许 图元重启
	inputAssembly.primitiveRestartEnable = false;

	//视口
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)WIDTH;
	viewport.height = (float)HEIGHT;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	//裁剪
	VkRect2D scissor = { };
	scissor.extent = swapChainExtent;
	scissor.offset = { 0,0 };

	//管线中 视口/裁剪 说明
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pViewports = &viewport;
	viewportState.viewportCount = 1;
	viewportState.pScissors = &scissor;
	viewportState.scissorCount = 1;

	//光栅器
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = false; //若为真，超过近、远平面的片段会被截断而不是丢弃
	rasterizer.rasterizerDiscardEnable = false; //若为真，所有片段都会被丢弃
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //多边形填充模式
	rasterizer.lineWidth = 1.0f; //线宽
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //面剔除模式
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //哪面为正面
	//使光栅器通过 增加一个常量/根据图元的斜率 来改变其深度值
	rasterizer.depthBiasEnable = false;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;

	//多重采样
	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = false;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = false;
	multisampleState.alphaToOneEnable = false;

	//深度缓冲

	//模板缓冲

	//混合
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.blendEnable = true; //是否开启blend
	// 哪些组成可以被写入
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	//混合方程中的各个因子
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

	//动态状态

	//管线布局
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; //描述符布局
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("Error : Failed to create pipeline layout!");

	//创建图形渲染管线
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pStages = shaderStageInfos; //shader stage
	pipelineInfo.stageCount = 2;
	pipelineInfo.pVertexInputState = &vertexInputInfo;  //顶点输入说明
	pipelineInfo.pInputAssemblyState = &inputAssembly;  //输入装配说明
	pipelineInfo.pViewportState = &viewportState;	    //视口说明
	pipelineInfo.pRasterizationState = &rasterizer;     //光栅器说明
	pipelineInfo.pMultisampleState = &multisampleState; //多重采样说明
	pipelineInfo.pDepthStencilState = nullptr;			//深度/模板说明
	pipelineInfo.pColorBlendState = &colorBlendState;	//颜色混合说明
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = pipelineLayout;				//管线布局
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;	//基础管线（vulkan允许在已经存在的管线上派生新的管线）
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		throw std::runtime_error("Error : Failed to create graphics pipeline!");
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

//创建framebuffer
void Demo::createFramebuffers()
{
	//为swapchain中的每一个image创建framebuffer
	swapChainFramebuffers.resize(swapChainImageViews.size());

	size_t i = 0;
	for (const auto& imageViews : swapChainImageViews)
	{
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass; //确定framebuffer将与哪个renderpass兼容
		framebufferInfo.pAttachments = &imageViews; //为framebuffer指定imageview
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1; //image数组中layer数量

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i++]) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create framebuffer!");
	}
}

//创建命令池
void Demo::createCommandPool()
{
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	commandPoolInfo.flags = 0;
	if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw std::runtime_error("Error : Failed to create command pool!");
}

//创建描述符集合
void Demo::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.pSetLayouts = layouts.data();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());

	descriptorSets.resize(swapChainImages.size());
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < swapChainImages.size(); ++i)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffers[i]; //描述符所引用的buffer
		bufferInfo.offset = 0; //偏移量
		bufferInfo.range = sizeof(UniformBufferObj); //buffer的范围

		//配置描述符
		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSets[i]; //将要修改的描述符
		descriptorWrite.dstBinding = 0; //与shader的binding对应
		descriptorWrite.dstArrayElement = 0; //要修改的描述符在descriptorSet中的索引
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //要修改的修饰符的类型
		descriptorWrite.descriptorCount = 1; //描述符的数量
		descriptorWrite.pBufferInfo = &bufferInfo; //描述符引用的buffer的信息
		descriptorWrite.pImageInfo = nullptr; //描述符引用image的信息（此处没有用到）
		descriptorWrite.pTexelBufferView = nullptr;

		//更新描述符集合(descriptorSets)
		//                             写入描述符           复制描述符
		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
	}
}

void Demo::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(decltype(vertices)::value_type)*vertices.size();
	//临时缓冲
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				 stagingBuffer, stagingBufferMemory);
	//内存映射
	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
	//复制缓冲区
	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	//清理临时缓冲
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

//创建uniform buffer
void Demo::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObj);

	uniformBuffers.resize(swapChainImages.size());
	uniformBuffersMemroy.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); ++i)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					 uniformBuffers[i], uniformBuffersMemroy[i]);
	}
}

//创建描述符池(descriptorPool)
void Demo::createDescriptorPool()
{
	//描述符池的大小
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size()); //每一个image都有一个描述符

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pPoolSizes = &poolSize; //描述符池大小
	poolInfo.poolSizeCount = 1; //描述符大小的数量
	poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

	//创建描述符池
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create descriptor pool!");
	}
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

	//为每一个swap chain framebuffer 申请命令缓冲
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //可以直接送入队列执行，但不能被其他command buffer调用
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("Error : Failed to allocate command buffers!");

	size_t size = commandBuffers.size();
	for (size_t i = 0; i < size; ++i)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //命令缓冲可以在等待执行时再次提交
		beginInfo.pInheritanceInfo = nullptr;

		//开始记录command
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

		//开始一个renderpass
		vkCmdBeginRenderPass(commandBuffers[i], &renderInfo, VK_SUBPASS_CONTENTS_INLINE);
		//为command buffer绑定pipeline
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		//绑定顶点缓冲
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		//绑定描述符
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
								0, //第一个描述符集合的索引 
								1, //描述符的数量
								&descriptorSets[i], //描述符数组
								0, nullptr);
		//绘制
		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		//结束renderpass
		vkCmdEndRenderPass(commandBuffers[i]);
		//结束一个command buffer
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to record command buffer!");
	}
}

void Demo::createSyncObjects()
{
	//VkSemaphore  用于同步一个或多个队列
	//VkFence      用于同步程序自身与渲染
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
			throw std::runtime_error("Error ：Failed to create semaphore!");
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create semaphore!");
		if (vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create fence!");
	}
}

//重新创建交换链
void Demo::recreateSwapChain()
{
	//等待设备执行完成
	vkDeviceWaitIdle(device);
	cleanupSwapChain();

	createSwapChain();
	createImageViews(); //image view基于swapchain中的image
	createRenderPass(); //render pass基于swapchain的format
	createGraphicsPipeline(); //graphics pipeline中设置了viewport & scissor
	createFramebuffers();
	createCommandBuffers();
}

//清理交换链
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

//检测验证层支持
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

//检测物理设备是否支持扩展
bool Demo::checkDeviceExtionSupport(VkPhysicalDevice & physicalDevice)
{
	//枚举设备扩展特性
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
{//获取交换链细节信息
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

//检测物理设备是否符合要求
bool Demo::isDeviceSuitable(VkPhysicalDevice & physicalDevice)
{
	//获取物理设备特性
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

	//检测物理设备特性是否符合要求
	bool devicePorpertiesSuit = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
		&& deviceFeatures.geometryShader;

	//检测物理设备队列簇是否符合要求
	bool queueFamilySuit = findQueueFamilies(physicalDevice).isComplete();

	//检测物理设备扩展是否支持
	bool deviceExtensionSuit = checkDeviceExtionSupport(physicalDevice);

	//检测交换链是否支持
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

//寻找合适的队列簇
QueueFamilyIndices Demo::findQueueFamilies(VkPhysicalDevice & physicalDevice)
{
	//获取物理设备队列簇特性
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	uint32_t i = 0;
	for (auto& queueFamily : queueFamilies)
	{
		//队列是否支持graphics
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		//队列是否支持present
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

// 获取需要的程序需要的扩展列表
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
	//创建缓冲
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create buffer!");
	}
	//获取内存需求
	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
	//分配内存
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to allocate memory!");
	}
	//绑定内存到缓冲区
	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void Demo::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	//创建临时command buffer
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	//记录command
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //此命令只会提交一次,并会阻塞

	//开始记录
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	//复制缓冲
	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	//结束记录
	vkEndCommandBuffer(commandBuffer);

	//提交command
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.commandBufferCount = 1;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue); //等待提交完成

	//释放临时command buffer
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}