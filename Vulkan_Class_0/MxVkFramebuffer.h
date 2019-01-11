#pragma once
#ifndef _MX_VK_FRAMEBUFFER_H_
#define _MX_VK_FRAMEBUFFER_H_

#include"MxVkManager.h"

#include<vector>

namespace Mixel
{
	class MxVkFramebuffer
	{
	private:
		bool mIsReady;

		const MxVkManager* mManager;
		VkFramebuffer mFramebuffer;
		VkRenderPass mRenderPass;
		VkExtent2D mExtent;
		uint32_t mLayers;

		std::vector<VkImageView>* mAttachments;

		void clear();
	public:
		MxVkFramebuffer();
		bool setup(const MxVkManager* manager);
		void setTargetRenderPass(const VkRenderPass renderPass) { mRenderPass = renderPass; };
		void setExtent(const VkExtent2D& extent) { mExtent = extent; };
		void setLayers(const uint32_t layer) { mLayers = layer; };
		void addAttachments(std::vector<VkImageView>& attachments);
		bool createFramebuffer();
		VkFramebuffer getFramebuffer() const { return mFramebuffer; };
		void destroy();
		~MxVkFramebuffer();
	};
}

#endif // !_MX_VK_FRAMEBUFFER_H_
