#pragma once
#ifndef _MX_VULKAN_SHADER_H_
#define _MX_VULKAN_SHADER_H_

#include"MxVulkanManager.h"

#include<fstream>
#include<vector>
#include<list>
#include<algorithm>
#include<string>

namespace Mixel
{
	struct MxVulkanShaderModule
	{
		VkShaderModule module;
		VkShaderStageFlagBits stage;
		bool operator==(const MxVulkanShaderModule& a)
		{
			return module == a.module;
		}
	};

	class MxVulkanShaderHelper
	{
	private:
		bool mIsReady;

		const MxVulkanManager* mManager;
		std::list<MxVulkanShaderModule> mModules;

	public:
		using ShaderModuleIterator = std::list<MxVulkanShaderModule>::const_iterator;
		MxVulkanShaderHelper() :mIsReady(false), mManager(nullptr) {};
		bool setup(const MxVulkanManager* manager);
		ShaderModuleIterator createModule(const std::vector<char>& code,const VkShaderStageFlagBits stage);
		ShaderModuleIterator createModule(const std::string& path, const VkShaderStageFlagBits stage);
		bool destroyModule(ShaderModuleIterator it);
		void destroy();
		~MxVulkanShaderHelper() { destroy(); };
	};

	bool MxVulkanShaderHelper::setup(const MxVulkanManager* manager)
	{
		if (mIsReady)
			destroy();

		mManager = manager;
		mIsReady = true;
		return true;
	}

	MxVulkanShaderHelper::ShaderModuleIterator MxVulkanShaderHelper::createModule(const std::vector<char>& code,const VkShaderStageFlagBits stage)
	{
		VkShaderModule temp;
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		MX_VK_CHECK_RESULT(vkCreateShaderModule(mManager->getDevice(), &createInfo, nullptr, &temp));
		mModules.push_back({ temp,stage });
		return --mModules.end();
	}

	MxVulkanShaderHelper::ShaderModuleIterator MxVulkanShaderHelper::createModule(const std::string & path, const VkShaderStageFlagBits stage)
	{
		std::ifstream inFile(path, std::ios_base::ate | std::ios_base::binary);
		if (!inFile.is_open())
			throw std::runtime_error("Error : Failed to open file");
		size_t fileSize = static_cast<size_t>(inFile.tellg());
		std::vector<char> buffer(fileSize);
		inFile.seekg(std::ios_base::beg);
		inFile.read(buffer.data(), fileSize);
		inFile.close();
		return createModule(buffer, stage);
	}

	bool MxVulkanShaderHelper::destroyModule(ShaderModuleIterator it)
	{
		auto find = std::find(mModules.begin(), mModules.end(), *it);
		if (find == mModules.end())
			return false;
		vkDestroyShaderModule(mManager->getDevice(), it->module, nullptr);
		mModules.erase(it);
	}

	void MxVulkanShaderHelper::destroy()
	{
		if (!mIsReady)
			return;
		for (auto& module : mModules)
		{
			vkDestroyShaderModule(mManager->getDevice(), module.module, nullptr);
		}
		mManager = nullptr;
		mIsReady = false;
	}
}

#endif // !_MX_VULKAN_SHADER_H_