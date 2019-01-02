#pragma once
#ifndef _MX_VULKAN_FRAMEBUFFER_H_
#define _MX_VULKAN_FRAMEBUFFER_H_

#include"MxVulkanManager.h"

#include<vector>

namespace Mixel
{
	class MxVulkanFramebuffer
	{
	private:
		bool mIsReady;

		const MxVulkanManager* mManager;
		VkFramebuffer mFramebuffer;
		VkRenderPass mRenderPass;
		VkExtent2D mExtent;
		uint32_t mLayers;

		std::vector<VkImageView>* mAttachments;

		void clear();
	public:
		MxVulkanFramebuffer();
		bool setup(const MxVulkanManager* manager);
		void setTargetRenderPass(const VkRenderPass renderPass) { mRenderPass = renderPass; };
		void setExtent(const VkExtent2D& extent) { mExtent = extent; };
		void setLayers(const uint32_t layer) { mLayers = layer; };
		void addAttachments(std::vector<VkImageView>& attachments);
		bool createFramebuffer();
		VkFramebuffer getFramebuffer() const { return mFramebuffer; };
		void destroy();
		~MxVulkanFramebuffer();
	};
}

#endif // !_MX_VULKAN_FRAMEBUFFER_H_
