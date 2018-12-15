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
	createCommandPool();
	createColorResource();
	createDepthResource();
	createFramebuffers();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
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
	vkDestroySampler(device, textureSampler, nullptr);
	vkDestroyImageView(device, textureImageView, nullptr);
	vkDestroyImage(device, textureImage, nullptr);
	vkFreeMemory(device, textureImageMemory, nullptr);
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
	} else if (result != VK_SUCCESS)
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
		throw std::runtime_error("Error : Failed to submit draw command buffer!");

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
	} else if (result != VK_SUCCESS)
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
	ubo.view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
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
	} else
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
			msaaSamples = getMaxUsableSampleCount();
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
	deviceFeatures.samplerAnisotropy = VK_TRUE; //开启各项异性过滤

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
	} else
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
	} else
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
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

//创建render pass
void Demo::createRenderPass()
{
	//多重采样颜色附件描述
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainFormat; //image view的格式
	colorAttachment.samples = msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //在subpass开始时对现有数据的操作
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //在subpass结束时是否对数据存储
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //对模板缓冲的操作
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //renderpass开始时image的布局
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //renderpass结束时image的布局

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0; //attachment的索引
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //附件被引用时的布局(layout)

	//深度/模板缓冲附件描述
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//显示颜色附件描述
	VkAttachmentDescription colorAttachmentResolve = {};
	colorAttachmentResolve.format = swapChainFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentResolveRef = {};
	colorAttachmentRef.attachment = 2;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//子通道描述
	//每个子通道会引用一个或多个 附件(attachment)
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //该subpass绑定到graphics管道
	subpass.pColorAttachments = &colorAttachmentRef; //颜色附件引用
	subpass.colorAttachmentCount = 1;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

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

	std::array<VkAttachmentDescription, 3>  attachments = { colorAttachment,depthAttachment,colorAttachmentResolve };

	//创建render pass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachments.data(); //renderpass所有的附件
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
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

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding bindings[] =
	{
		uboLayoutBinding, samplerLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pBindings = bindings;
	layoutInfo.bindingCount = 2;

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
	multisampleState.sampleShadingEnable = true;
	multisampleState.rasterizationSamples = msaaSamples;
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = false;
	multisampleState.alphaToOneEnable = false;

	//深度/模板缓冲
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE; //用于控制在范围内的图形通过测试
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 0.0f;

	depthStencil.stencilTestEnable = VK_FALSE; //模板测试 TODO：自己弄吧，教程里面没有(￣_￣|||)
	depthStencil.front = {};
	depthStencil.back = {};

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
	pipelineInfo.pStages = shaderStageInfos;			//shader stage
	pipelineInfo.stageCount = 2;
	pipelineInfo.pVertexInputState = &vertexInputInfo;  //顶点输入说明
	pipelineInfo.pInputAssemblyState = &inputAssembly;  //输入装配说明
	pipelineInfo.pViewportState = &viewportState;	    //视口说明
	pipelineInfo.pRasterizationState = &rasterizer;     //光栅器说明
	pipelineInfo.pMultisampleState = &multisampleState; //多重采样说明
	pipelineInfo.pDepthStencilState = &depthStencil;	//深度/模板说明
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

	for (size_t i = 0; i < swapChainFramebuffers.size(); ++i)
	{
		std::array<VkImageView, 3> attachments = { colorImageView, depthImageView, swapChainImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass; //确定framebuffer将与哪个renderpass兼容
		framebufferInfo.pAttachments = attachments.data(); //为framebuffer指定imageview
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1; //image数组中layer数量

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
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
	{
		throw std::runtime_error("Error : Failed to create command pool!");
	}
}

void Demo::createColorResource()
{
	VkFormat colorFormat = swapChainFormat;

	createImage(swapChainExtent.width, swapChainExtent.height, msaaSamples, colorFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				colorImage, colorImageMemory);

	colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);

	transitionImageLayout(colorImage, colorFormat,
						  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

//创建深度缓冲区
void Demo::createDepthResource()
{
	VkFormat depthFormat = findDepthFormat();

	createImage(swapChainExtent.width, swapChainExtent.height, msaaSamples,
				depthFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

//创建texture image
void Demo::createTextureImage()
{
	//读取texture
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("Image/Hz.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels)
	{
		throw std::runtime_error("Error : Failed to load texture image!");
	}

	//创建临时缓冲
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				 stagingBuffer, stagingBufferMemory);
	//复制数据
	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, (size_t)imageSize);
	vkUnmapMemory(device, stagingBufferMemory);

	//释放原texture数据
	stbi_image_free(pixels);

	//创建image
	createImage(texWidth, texHeight, VK_SAMPLE_COUNT_1_BIT,
				VK_FORMAT_R8G8B8A8_SNORM, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM,
						  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer, textureImage,
					  static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Demo::createTextureImageView()
{
	/*VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device, &viewInfo, nullptr, &textureImageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create image view!");
	}*/

	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Demo::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE; //各向异性
	samplerInfo.maxAnisotropy = 16; //最大各向异性采样数
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; //边缘颜色(不透明的黑色)
	samplerInfo.unnormalizedCoordinates = VK_FALSE; //是否采样图片原坐标([0,width],[0,height])
	samplerInfo.compareEnable = VK_FALSE; //是否开启比较(如果开启，纹理会先与一个值进行比较)
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; //比较所使用的运算符
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; //mip贴图相关
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create sampler!");
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
	std::array<VkDescriptorPoolSize, 2> poolSizes;
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size()); //每一个image都有一个描述符
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pPoolSizes = poolSizes.data(); //描述符池大小
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size()); //描述符大小的数量
	poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

	//创建描述符池
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create descriptor pool!");
	}
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

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		//配置描述符
		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i]; //将要修改的描述符
		descriptorWrites[0].dstBinding = 0; //与shader的binding对应
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //要修改的修饰符的类型
		descriptorWrites[0].descriptorCount = 1; //描述符的数量
		descriptorWrites[0].pBufferInfo = &bufferInfo; //描述符引用的buffer的信息
		descriptorWrites[0].pImageInfo = nullptr; //描述符引用image的信息（此处没有用到）
		descriptorWrites[0].pTexelBufferView = nullptr;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i]; //将要修改的描述符
		descriptorWrites[1].dstBinding = 1; //与shader的binding对应
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; //要修改的修饰符的类型
		descriptorWrites[1].descriptorCount = 1; //描述符的数量
		descriptorWrites[1].pBufferInfo = nullptr;
		descriptorWrites[1].pImageInfo = &imageInfo;
		descriptorWrites[1].pTexelBufferView = nullptr;

		//更新描述符集合(descriptorSets)
		//                             写入描述符           复制描述符
		vkUpdateDescriptorSets(device,
							   static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
							   0, nullptr);
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

		std::array<VkClearValue, 3> clearValues = {};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f,0 };
		clearValues[2].color = { 0.0f, 0.0f, 0.0f, 1.0f };

		VkRenderPassBeginInfo renderInfo = {};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderInfo.renderPass = renderPass;
		renderInfo.framebuffer = swapChainFramebuffers[i];
		renderInfo.renderArea.offset = { 0,0 };
		renderInfo.renderArea.extent = swapChainExtent;

		renderInfo.pClearValues = clearValues.data();
		renderInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

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
	createColorResource();
	createDepthResource();
	createFramebuffers();
	createCommandBuffers();
}

//清理交换链
void Demo::cleanupSwapChain()
{
	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);

	vkDestroyImageView(device, colorImageView, nullptr);
	vkDestroyImage(device, colorImage, nullptr);
	vkFreeMemory(device, colorImageMemory, nullptr);

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

	//检测物理设备功能是否支持
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

	//检测交换链是否支持
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
	bool swapChainSuit = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

	if (queueFamilySuit && deviceExtensionSuit && swapChainSuit && supportedFeatures.samplerAnisotropy)
	{
		std::cout << std::endl << "[ Physical device ] :" << std::endl
			<< "\tDevice name : " << deviceProperties.deviceName << std::endl;
		return true;
	} else
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
	} else
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

void Demo::createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples,
					   VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
					   VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & memory)
{
	//创建texture image
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D; //image类型
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1; //extent可以代表长宽高(image可以是3D的)
	imageInfo.mipLevels = 1; //mip贴图层次
	imageInfo.arrayLayers = 1; //image的层数
	imageInfo.format = format;

	//VK_IMAGE_TILING_OPTIMAL image以一个特定的布局存放以保证后续(shader)存取的高效
	//VK_IMAGE_TILING_LINEAR  image以行优先的方式存储，可以直接存取
	imageInfo.tiling = tiling; //image在内存中的布局 
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //在第一个变换执行时GPU不能使用，并且丢弃原有数据

	//image将会被用作传输数据的目标，也将被用于shader采样
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //image将只被一个队列簇使用
	imageInfo.samples = numSamples; //用于多重采样的数量，此处只有一个image

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create image!");
	}

	//获取image内存需求
	VkMemoryRequirements memRequirements = {};
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	//申请内存
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to allocate image memory!");
	}

	vkBindImageMemory(device, image, memory, 0);
}

VkImageView Demo::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Error : Failed to create image view!");
	}

	return imageView;
}

void Demo::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	//创建临时command buffer
	VkCommandBuffer commandBuffer = beginTempCommands();

	//复制缓冲
	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	//结束记录
	endTempCommands(commandBuffer);
}

//开始一个临时命令缓冲
VkCommandBuffer Demo::beginTempCommands()
{
	//分配临时command buffer
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
	//开始记录command buffer
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

//结束一个临时命令缓冲
void Demo::endTempCommands(VkCommandBuffer commandBuffer)
{
	//结束command buffer的记录
	vkEndCommandBuffer(commandBuffer);

	//提交
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.commandBufferCount = 1;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	//释放临时command buffer
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

//转换image布局
void Demo::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginTempCommands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout; //旧的布局
	barrier.newLayout = newLayout; //新的布局
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //当需要转移所有权(属于哪个队列簇)的时候
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //填入队列簇的索引
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags srcStage; //转换之前管线状态
	VkPipelineStageFlags dstStage; //转换之后管线状态

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{//image被用作深度/模板缓冲
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (hasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{//image被用作转移的目标
		barrier.srcAccessMask = 0; //源存取方式
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; //目标存取方式

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; //之前的管线状态：管线开始
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT; //之后的管线状态：转移
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{//image被用作贴图
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			   newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	} else
	{
		throw std::runtime_error("Error : Unsupported transistion!");
	}

	vkCmdPipelineBarrier(commandBuffer,
						 srcStage, dstStage,
						 0,
						 0, nullptr, //memory barrier
						 0, nullptr, //buffer memory barrier
						 1, &barrier); //image barrier
	endTempCommands(commandBuffer);
}

void Demo::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginTempCommands();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0; //像素在内存中的排布如果不是紧密排布则需要设置
	region.bufferImageHeight = 0; //同上

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0,0,0 };
	region.imageExtent = { width,height,1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,//image现在的布局
						   1, &region);

	endTempCommands(commandBuffer);
}

VkFormat Demo::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (const VkFormat& format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		} else
		{
			throw std::runtime_error("Error : Failed to find supported format!");
		}
	}
}

VkSampleCountFlagBits Demo::getMaxUsableSampleCount()
{
	static VkSampleCountFlagBits sampleCountFlags[] = {
		VK_SAMPLE_COUNT_64_BIT,
		VK_SAMPLE_COUNT_32_BIT,
		VK_SAMPLE_COUNT_16_BIT,
		VK_SAMPLE_COUNT_8_BIT,
		VK_SAMPLE_COUNT_4_BIT,
		VK_SAMPLE_COUNT_2_BIT,
		VK_SAMPLE_COUNT_1_BIT
	};

	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = std::min(physicalDeviceProperties.limits.framebufferColorSampleCounts,
										 physicalDeviceProperties.limits.framebufferDepthSampleCounts);

	for (auto& sampleCount : sampleCountFlags)
	{
		if (counts & sampleCount)
			return sampleCount;
	}
	return VK_SAMPLE_COUNT_1_BIT;
}

VkFormat Demo::findDepthFormat()
{
	return findSupportedFormat(
		{
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool Demo::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
