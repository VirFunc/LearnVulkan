#pragma once
#ifndef _MX_VULKAN_DESCRIPTOR_H_
#define _MX_VULKAN_DESCRIPTOR_H_

#include"MxVulkanManager.h"

#include<map>
#include<vector>

namespace Mixel
{
	class MxVulkanDescriptorSetLayout
	{
	private:
		bool mIsReady;
		const MxVulkanManager* mManager;
		VkDescriptorSetLayout mLayout;
		std::vector<VkDescriptorSetLayoutBinding> mBindings;

		void clear() { mBindings.clear(); }
	public:
		MxVulkanDescriptorSetLayout();
		bool setup(const MxVulkanManager* manager);
		void addBindings(uint32_t binding, VkDescriptorType type, uint32_t count,
						 VkShaderStageFlags stage,
						 const VkSampler* immutableSamplers = nullptr);
		bool createDescriptorSetLayout();
		VkDescriptorSetLayout getDescriptorSetLayout() const { return mLayout; };
		void destroy();
		~MxVulkanDescriptorSetLayout() { destroy(); }
	};

	class MxVulkanDescriptorPool
	{
	private:
		bool mIsReady;

		const MxVulkanManager* mManager;
		VkDescriptorPool mDescriptorPool;

		std::map<VkDescriptorType, uint32_t> mPoolSizes;

	public:
		MxVulkanDescriptorPool();

		bool setup(const MxVulkanManager* manager);
		void addPoolSize(VkDescriptorType type, uint32_t count);
		bool createDescriptorPool(uint32_t maxSets);
		VkDescriptorPool getDescriptorPool() const { return mDescriptorPool; }

		std::vector<VkDescriptorSet> allocDescriptorSet(const std::vector<VkDescriptorSetLayout>& layout);
		std::vector<VkDescriptorSet> allocDescriptorSet(const VkDescriptorSetLayout layout, const uint32_t count);
		VkDescriptorSet allocDescriptorSet(const VkDescriptorSetLayout layout);

		std::vector<VkDescriptorSet> allocDescriptorSet(const std::vector<MxVulkanDescriptorSetLayout>& layout);
		std::vector<VkDescriptorSet> allocDescriptorSet(const MxVulkanDescriptorSetLayout& layout, const uint32_t count);;
		VkDescriptorSet allocDescriptorSet(const MxVulkanDescriptorSetLayout& layout);
		~MxVulkanDescriptorPool();
		void destroy();
	};
}
#endif // !_MX_VULKAN_DESCRIPTOR_H_