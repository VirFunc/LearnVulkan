#pragma once
#include"util.hpp"
#define MX_DEBUG _DEBUG
#include<vulkan/vulkan.h>

#define SDL_MAIN_HANDLED
#include<SDL2/SDL.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include<glm/glm.hpp>

#include<stb/stb_image.h>

#include<iostream>
#include<string>
#include<chrono>
#include<array>
#include<vector>

class VulkanRender
{
private:


protected:
	SDL_Window* window;
	size_t currFram;
	bool paused;
	bool framebufferResized;

	//vulkan
	struct Vulkan
	{
		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice device;		
		VkSurfaceKHR surface;

		VkPhysicalDeviceFeatures enableFeatures;

		struct
		{
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
			
			struct
			{
				VkSurfaceCapabilitiesKHR capabilities;
				std::vector<VkSurfaceFormatKHR> formats;
				std::vector<VkPresentModeKHR> presentModes;
			}swapchainSupport;

		}properties;

		struct
		{
			std::vector<const char*> deviceExt;
			std::vector<const char*> instanceExt;
		}enabledExtensions;

		struct
		{
			VkQueue graphicsQueue;
			VkQueue presentQueue;
		}queueComp;

		struct
		{
			VkSwapchainKHR swapchain;
			std::vector<SwapchainImage> images;
			std::vector<VkFramebuffer> framebuffers;
			VkFormat format;
			VkExtent2D extent;
		}swapchainComp;

		struct
		{
			VkPipeline pipeline;
			VkPipelineLayout layout;
		}pipelineComp;

		VkRenderPass renderPass;

		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;

		VkCommandPool commandPool;

		Image depthImage;
	}mVulkan;

	void initVulkan();
	void createInstance();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapchain();


	void clearSwapChain();

public:
	VulkanRender();
	VulkanRender(const SDL_Window* window) { bindToWindow(window); };
	void bindToWindow(const SDL_Window* window);
	virtual bool initialize();
	virtual void destroy();
	~VulkanRender();

	struct
	{
		bool enabled;
		std::vector<const char*> layers;
		bool checkSupport();
	}validationLayers;
};

