/*************************************************/
/*这是learnVulkan的第四个项目*/
/*基于SDL2*/
/*这个项目将会加入Uniform buffers*/
/*将会使用Resource descriptors(资源描述符)*/
/*************************************************/
#pragma once
#include<vulkan/vulkan.h>
#define SDL_MAIN_HANDLED
#include<SDL2/SDL.h>
#include<SDL2/SDL_vulkan.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp> //矩阵变换

#include<iostream>
#include<fstream>
#include<string>
#include<stdexcept>
#include<functional>
#include<cstdlib>
#include<vector>
#include<optional>
#include<set>
#include<limits>
#include<algorithm>
#include<array>
#include<chrono>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily = -1;
	std::optional<uint32_t> presentFamily = -1;

	bool isComplete()
	{
		return graphicsFamily.value() >= 0 && presentFamily.value() >= 0;
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;

	//返回Vertex对于的顶点绑定(VertexInputBinding)描述
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0; //顶点数据的绑定点
		bindingDescription.stride = sizeof(Vertex); //两个Vertex之间的间隔
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	//返回顶点数据中每个 属性的描述 
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescription = {};
		attributeDescription[0].binding = 0; //顶点数据的绑定点
		attributeDescription[0].location = 0; //在vertex shader中的location
		attributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT; //属性的数据格式
		attributeDescription[0].offset = offsetof(Vertex, pos); //属性相对于一个Vertex起始位置的便宜量

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, color);

		return attributeDescription;
	}
};

struct UniformBufferObj
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;
#define MAX_FRAMES_IN_FLIGHT 2

const std::vector<Vertex> vertices =
{
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices =
{
	0, 1, 2, 2, 3, 0
};

//将打开的验证层
const std::vector<const char*> validationLayers =
{
	"VK_LAYER_LUNARG_standard_validation"
};

//将开启的设备扩展
const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef _DEBUG
constexpr bool enableValidationLayers = true;
#else
constexpr bool enableValidationLayers = false;
#endif

VkResult createDebugUtilsMessengerEXT(VkInstance instance,
									  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
									  const VkAllocationCallbacks* pAllocator,
									  VkDebugUtilsMessengerEXT* pCallback);

void destroyDebugUtilsMessengerEXT(VkInstance instance,
								   VkDebugUtilsMessengerEXT callback,
								   const VkAllocationCallbacks* pAllocator);


class Demo
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
	Demo();
	~Demo();
private:
	SDL_Window* window = nullptr;
	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT callback = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	QueueFamilyIndices queueFamilyIndices;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkFormat swapChainFormat;
	VkExtent2D swapChainExtent;

	//framebuffer 附件相关信息
	//被使用的 颜色/深度缓冲区/采样器 的数目
	//整个渲染操作中对内容的处理过程
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores; //表示得到image可以进行渲染
	std::vector<VkSemaphore> renderFinishedSemaphores; //表示渲染结束可以提交进行显示
	std::vector<VkFence> inFlightFences;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemroy;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	size_t currFrame = 0;
	bool framebufferResized = false;

	void initWindow();
	void initVulkan();
	void mainLoop();
	void cleanup();

	void drawFrame();
	void updateUniformBuffer(uint32_t currImage);

	void createInstance();
	void setupDebugCallback();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createDescriptorSets();
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffers();
	void createDescriptorPool();
	void createCommandBuffers();
	void createSyncObjects();

	void recreateSwapChain();
	void cleanupSwapChain();

	bool checkValidationLayerSupport();
	bool checkDeviceExtionSupport(VkPhysicalDevice& physicalDevice);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice& device);
	bool isDeviceSuitable(VkPhysicalDevice& physicalDevice);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice& physicalDevice);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& avaliableFormats);
	VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR>& avaliablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capacities);
	std::vector<const char*> getRequiredExtensions();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
					  VkMemoryPropertyFlags properties, VkBuffer& buffer,
					  VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	//vulkan debug callback
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	static std::vector<char> readFile(const std::string& path);
};