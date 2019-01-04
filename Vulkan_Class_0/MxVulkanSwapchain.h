#pragma once
#ifndef _MX_VULKAN_SWAPCHAIN_H_
#define _MX_VULKAN_SWAPCHAIN_H_

#include"MxVulkanImage.h"
#include"MxVulkanManager.h"

#include<vector>
#include<algorithm>

namespace Mixel
{
	class MxVulkanSwapchain
	{
	private:
		bool mIsReady;
		const MxVulkanManager* mManager;

		VkSwapchainKHR mSwapchain;
		VkSurfaceFormatKHR mCurrFormat;
		VkPresentModeKHR mCurrPresentMode;
		VkExtent2D mCurrExtent;

		std::vector<VkImage> mSwapChainImages;
		std::vector<VkImageView> mSwapChainImageViews;

		struct SwapchainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		}mSwapchainSupportDetails;

		SwapchainSupportDetails getSwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
		bool chooseFormat(const std::vector<VkSurfaceFormatKHR>& rqFormats, VkSurfaceFormatKHR& format);
		bool choosePresentMode(const VkPresentModeKHR rqPresentMode, VkPresentModeKHR& presentMode);
		void createSwapchainImageView();
		VkExtent2D chooseExtent(const VkExtent2D& rqExtent);

	public:
		MxVulkanSwapchain();
		bool setup(const MxVulkanManager* manager);
		std::vector<VkSurfaceFormatKHR> getSupportFormat() const;
		std::vector<VkPresentModeKHR> getSupportPresentMode() const;
		bool createSwapchain(const std::vector<VkSurfaceFormatKHR>& rqFormats,
							 VkPresentModeKHR rqPresentMode,
							 VkExtent2D rqExtent);
		const VkSurfaceFormatKHR& getCurrFormat() const { return mCurrFormat; };
		VkPresentModeKHR getCurrPresentMode() const { return mCurrPresentMode; };
		const VkExtent2D& getCurrExtent() const { return mCurrExtent; };
		uint32_t getImageCount() const { return mSwapChainImages.size(); };
		const std::vector<VkImageView> getImageViews() const { return mSwapChainImageViews; };
		void destroy();
		~MxVulkanSwapchain();
	};
}

#endif