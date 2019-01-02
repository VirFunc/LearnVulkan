#include "MxVulkanFramebuffer.h"

namespace Mixel
{
	void MxVulkanFramebuffer::clear()
	{
		if (mAttachments)
			delete mAttachments;
	}
	MxVulkanFramebuffer::MxVulkanFramebuffer() :mIsReady(false), mManager(nullptr), mRenderPass(VK_NULL_HANDLE)
	{
	}

	bool MxVulkanFramebuffer::setup(const MxVulkanManager * manager)
	{
		if (mIsReady)
			destroy();

		mAttachments = new std::vector<VkImageView>;
		if (!mAttachments)
			return false;
		mManager = manager;
		mIsReady = true;
		return true;
	}

	void MxVulkanFramebuffer::addAttachments(std::vector<VkImageView>& attachments)
	{
		mAttachments->insert(mAttachments->end(), attachments.begin(), attachments.end());
	}

	bool MxVulkanFramebuffer::createFramebuffer()
	{
		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = mRenderPass;
		createInfo.pAttachments = mAttachments->data();
		createInfo.attachmentCount = mAttachments->size();
		createInfo.width = mExtent.width;
		createInfo.height = mExtent.height;
		createInfo.layers = mLayers;

		if (vkCreateFramebuffer(mManager->getDevice(), &createInfo, nullptr, &mFramebuffer) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create framebuffer");
		return true;
	}

	void MxVulkanFramebuffer::destroy()
	{
		if (!mIsReady)
			return;

		if (mAttachments)
			delete mAttachments;
		vkDestroyFramebuffer(mManager->getDevice(), mFramebuffer, nullptr);
		mAttachments = nullptr;
		mManager = nullptr;
		mRenderPass = VK_NULL_HANDLE;
		mFramebuffer = VK_NULL_HANDLE;
		mExtent = { 0,0 };
		mLayers = 0;
	}

	MxVulkanFramebuffer::~MxVulkanFramebuffer()
	{
		destroy();
	}
}
