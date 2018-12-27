#pragma once
#ifndef _MX_VULKAN_INITIALIZER_H_
#define _MX_VULKAN_INITIALIZER_H_

#include<vulkan/vulkan.h>

#define SDL_MAIN_HANDLED
#include<SDL2/SDL.h>
#include<SDL2/SDL_vulkan.h>

#include"MxVulkanUtils.h"

#include<iostream>
#include<vector>
#include<string>
#include<map>

namespace Mixel
{
	class MxVulkanManager
	{
	public:
		struct InitializeInfo
		{
			bool debugMode;
			bool present;

			struct
			{
				std::vector<const char*> validationLayers;
				struct
				{
					std::string appName;
					std::string engineName;
					uint32_t engineVersion;
					uint32_t appVersion;
				}appInfo;
				std::vector<const char*> extensions;
			}instance;

			SDL_Window* window;

			struct
			{
				struct
				{
					VkPhysicalDeviceType type;
					std::vector<const char*> extensions;
					VkQueueFlags queueFlags;
					struct
					{
						float graphics;
						float compute;
						float present;
					}queuePrioriy;
					VkPhysicalDeviceFeatures enabledFeatures;
				}physical;
			}device;
		};

	private:
		bool mIsReady;

		VkSurfaceKHR mSurface;
		struct
		{
			VkDevice logicalDevice;
			VkPhysicalDevice physicalDevice;
			VkPhysicalDeviceProperties properties;
			VkPhysicalDeviceFeatures features;
			std::vector<VkExtensionProperties> extensionSupported;
		}mDevice;

		struct
		{
			VkInstance instance;
			std::vector<VkExtensionProperties> extensionSupported;
		}mInstance;

		MxVulkanQueueFamilyIndices mQueueFamilyIndices;

		void createInstance(const InitializeInfo & info);
		void createSurface(const InitializeInfo & info); //todo depart this
		void pickPhysicalDevice(const InitializeInfo & info);
		void createLogicalDevice(const InitializeInfo & info);

		bool checkValidationLayerSupport(const InitializeInfo & info);
		bool isDeviceSuitable(const InitializeInfo & info, const VkPhysicalDevice& device);
		bool checkExtensions(const InitializeInfo& info, const VkPhysicalDevice& device);
		bool findQueueFamilies(const InitializeInfo& info, VkPhysicalDevice device, MxVulkanQueueFamilyIndices* indices);
		std::vector<VkExtensionProperties> getInstanceExtensions();
		std::vector<VkExtensionProperties> getDeviceExtensions(VkPhysicalDevice device);

	public:
		MxVulkanManager();

		void initialize(const InitializeInfo& info);
		void destroy();
		InitializeInfo* getEmptyInitInfo() const;
		const VkInstance& getInstance() const { return mInstance.instance; };
		const VkDevice& getDevice() const { return mDevice.logicalDevice; };
		const VkPhysicalDevice& getPhysicalDevice() const { return mDevice.physicalDevice; };
		const VkSurfaceKHR& getSurface() const { return mSurface; };
		const MxVulkanQueueFamilyIndices& getQueueFamilyIndices() const { return mQueueFamilyIndices; };

		/*MxVulkanDebug* createVulkanDebug() const;*/

		~MxVulkanManager() { destroy(); };
	};
}
#endif // !_MX_VULKAN_INITIALIZER_H_
