#include "MxVulkanDescriptor.h"

namespace Mixel
{
	MxVulkanDescriptorPool::MxVulkanDescriptorPool() :mIsReady(false), mManager(nullptr),
		mDescriptorPool(VK_NULL_HANDLE)
	{
	}

	bool MxVulkanDescriptorPool::setup(const MxVulkanManager * manager)
	{
		if (mIsReady)
			destroy();

		mManager = manager;
		mIsReady = true;
		return true;
	}

	void MxVulkanDescriptorPool::addPoolSize(VkDescriptorType type, uint32_t count)
	{
		if (mPoolSizes.count(type) == 0)
			mPoolSizes[type] = 1;
		else
			mPoolSizes[type] += count;
	}

	bool MxVulkanDescriptorPool::createDescriptorPool(uint32_t maxSets)
	{
		std::vector<VkDescriptorPoolSize> poolSizes;
		poolSizes.reserve(mPoolSizes.size());

		for (const auto& size : mPoolSizes)
		{
			poolSizes.push_back({ size.first, size.second });
		}

		VkDescriptorPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.pPoolSizes = poolSizes.data();
		createInfo.poolSizeCount = poolSizes.size();
		createInfo.maxSets = maxSets;

		if (vkCreateDescriptorPool(mManager->getDevice(), &createInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create descriptor pool");

		mPoolSizes.clear();
		return true;
	}

	std::vector<VkDescriptorSet> MxVulkanDescriptorPool::allocDescriptorSet(const std::vector<VkDescriptorSetLayout>& layouts)
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.pSetLayouts = layouts.data();
		allocInfo.descriptorSetCount = layouts.size();

		std::vector<VkDescriptorSet> tempSets(layouts.size());
		if (vkAllocateDescriptorSets(mManager->getDevice(), &allocInfo, tempSets.data()) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to allocate descriptor set");

		return tempSets;
	}

	VkDescriptorSet MxVulkanDescriptorPool::allocDescriptorSet(const VkDescriptorSetLayout layout)
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.pSetLayouts = &layout;
		allocInfo.descriptorSetCount = 1;

		VkDescriptorSet tempSet;
		if (vkAllocateDescriptorSets(mManager->getDevice(), &allocInfo, &tempSet) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to allocate descriptor set");

		return tempSet;
	}

	std::vector<VkDescriptorSet> MxVulkanDescriptorPool::allocDescriptorSet(const VkDescriptorSetLayout layout, const uint32_t count)
	{
		std::vector<VkDescriptorSetLayout> layouts(count, layout);
		return allocDescriptorSet(layouts);
	}

	std::vector<VkDescriptorSet> MxVulkanDescriptorPool::allocDescriptorSet(const std::vector<MxVulkanDescriptorSetLayout>& layouts)
	{
		std::vector<VkDescriptorSetLayout> tempLayouts;
		tempLayouts.reserve(layouts.size());
		for (const auto& layout : layouts)
			tempLayouts.push_back(layout.getDescriptorSetLayout());
		return allocDescriptorSet(tempLayouts);
	}

	VkDescriptorSet MxVulkanDescriptorPool::allocDescriptorSet(const MxVulkanDescriptorSetLayout & layout)
	{
		return allocDescriptorSet(layout.getDescriptorSetLayout());
	}

	std::vector<VkDescriptorSet> MxVulkanDescriptorPool::allocDescriptorSet(const MxVulkanDescriptorSetLayout & layout, const uint32_t count)
	{
		return allocDescriptorSet(layout.getDescriptorSetLayout(), count);
	}

	MxVulkanDescriptorPool::~MxVulkanDescriptorPool()
	{
		destroy();
	}

	void MxVulkanDescriptorPool::destroy()
	{
		if (!mIsReady)
			return;
		vkDestroyDescriptorPool(mManager->getDevice(), mDescriptorPool, nullptr);
		mManager = nullptr;
		mIsReady = false;
	}

	MxVulkanDescriptorSetLayout::MxVulkanDescriptorSetLayout() :mIsReady(false), mManager(nullptr), mLayout(VK_NULL_HANDLE)
	{
	}

	bool MxVulkanDescriptorSetLayout::setup(const MxVulkanManager * manager)
	{
		if (mIsReady)
			destroy();

		mManager = manager;
		mIsReady = true;
		return true;
	}

	void MxVulkanDescriptorSetLayout::addBindings(uint32_t binding, VkDescriptorType type, uint32_t count, VkShaderStageFlags stage, const VkSampler * immutableSamplers)
	{
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = type;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stage;
		layoutBinding.pImmutableSamplers = immutableSamplers;
	}

	bool MxVulkanDescriptorSetLayout::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pBindings = mBindings.data();
		layoutInfo.bindingCount = mBindings.size();

		VkDescriptorSetLayout layout;
		if (vkCreateDescriptorSetLayout(mManager->getDevice(), &layoutInfo, nullptr, &layout) != VK_SUCCESS)
		{
			throw std::runtime_error("Error : Failed to create descriptor set layout!");
		}
		mLayout = layout;
		return true;
	}

	void MxVulkanDescriptorSetLayout::destroy()
	{
		if (!mIsReady)
			return;

		clear();
		mManager = nullptr;
		mIsReady = false;
	}

}