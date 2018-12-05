/*************************************************/
/*����learnVulkan�ĵ�������Ŀ*/
/*����SDL2*/
/*�����Ŀ��������һ����Ŀ�Ļ�����*/
/*������Ȼ���*/
/*************************************************/
#pragma once
#include<vulkan/vulkan.h>
#define SDL_MAIN_HANDLED
#include<SDL2/SDL.h>
#include<SDL2/SDL_vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp> //����任
#include"stb_image.h"

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
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	//����Vertex���ڵĶ����(VertexInputBinding)����
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0; //�������ݵİ󶨵�
		bindingDescription.stride = sizeof(Vertex); //����Vertex֮��ļ��
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	//���ض���������ÿ�� ���Ե����� 
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescription = {};
		attributeDescription[0].binding = 0; //�������ݵİ󶨵�
		attributeDescription[0].location = 0; //��vertex shader�е�location
		attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT; //���Ե����ݸ�ʽ
		attributeDescription[0].offset = offsetof(Vertex, pos); //���������һ��Vertex��ʼλ�õı�����

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, color);

		attributeDescription[2].binding = 0;
		attributeDescription[2].location = 2;
		attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[2].offset = offsetof(Vertex, texCoord);

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
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	{{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
	{{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
	{{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};

const std::vector<uint16_t> indices =
{
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
};

//���򿪵���֤��
const std::vector<const char*> validationLayers =
{
	"VK_LAYER_LUNARG_standard_validation"
};

//���������豸��չ
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

	//framebuffer ���������Ϣ
	//��ʹ�õ� ��ɫ/��Ȼ�����/������ ����Ŀ
	//������Ⱦ�����ж����ݵĴ�������
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores; //��ʾ�õ�image���Խ�����Ⱦ
	std::vector<VkSemaphore> renderFinishedSemaphores; //��ʾ��Ⱦ���������ύ������ʾ
	std::vector<VkFence> inFlightFences;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemroy;
	VkImage textureImage;
	VkImageView textureImageView;
	VkDeviceMemory textureImageMemory;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	VkSampler textureSampler;
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
	void createCommandPool();
	void createDepthResource();
	void createFramebuffers();
	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
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
	void createImage(uint32_t width, uint32_t height,
					 VkFormat format, VkImageTiling tiling,
					 VkImageUsageFlags usage,
					 VkMemoryPropertyFlags properties,
					 VkImage& image, VkDeviceMemory& memory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginTempCommands();
	void endTempCommands(VkCommandBuffer commandBuffer);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
								 VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	bool hasStencilComponent(VkFormat format);

	//vulkan debug callback
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	static std::vector<char> readFile(const std::string& path);
};