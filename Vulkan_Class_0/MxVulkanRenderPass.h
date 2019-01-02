#pragma once
#ifndef _MX_VULKAN_RENDER_PASS_H_
#define _MX_VULKAN_RENDER_PASS_H_

#include"MxVulkanManager.h"

#include<vector>
#include<initializer_list>

namespace Mixel
{
	class MxVulkanRenderPass
	{
	private:
		bool mIsReady;

		const MxVulkanManager* mManager;
		VkRenderPass mRenderPass;

		struct SubpassContent
		{
			VkPipelineBindPoint bindPoint;
			std::vector<VkAttachmentReference> colorRef;
			std::vector<VkAttachmentReference> inputRef;
			std::vector<VkAttachmentReference> preserveRef;
			bool hasDepthStencilRef = false;
			VkAttachmentReference depthStencilRef;
			bool hasResolveRef = false;
			VkAttachmentReference resolveRef;
		};

		std::vector<VkAttachmentDescription>* mAttachments;
		std::vector<VkAttachmentReference>* mAttachRefs;
		std::vector<VkSubpassDependency>* mDependencies;
		std::vector<SubpassContent>* mSubpasses;

		void clear();
	public:
		MxVulkanRenderPass();
		bool setup(const MxVulkanManager* manager);
		size_t addColorAttach(VkFormat format, VkSampleCountFlagBits sampleCount,
							  VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
							  VkImageLayout initLayout, VkImageLayout finalLayout);
		size_t addDepthStencilAttach(VkFormat format, VkSampleCountFlagBits sampleCount,
									 VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
									 VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
									 VkImageLayout initLayout = VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		size_t addColorAttachRef(size_t index, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		size_t addDepthStencilAttachRef(size_t index, VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		size_t addSubpass(VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
		bool addSubpassColorRef(size_t subpassIndex, const std::vector<size_t>& refIndices);
		bool addSubpassDepthStencilRef(size_t subpassIndex, size_t refIndex);
		bool addSubpassResolveRef(size_t subpassIndex, size_t refIndex);
		size_t addDependency(size_t srcSubpass, size_t dstSubpass,
							 VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
							 VkAccessFlags srcAccess, VkAccessFlags dstAccess);
		bool createRenderPass();
		void destroy();
		~MxVulkanRenderPass();

	};
}

#endif // !_MX_VULKAN_RENDER_PASS_H_

