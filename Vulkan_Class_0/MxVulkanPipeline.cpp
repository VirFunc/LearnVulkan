#include "MxVulkanPipeline.h"

namespace Mixel
{
	MxVulkanPipeline::MxVulkanPipeline() :mIsReady(false), mManager(nullptr),
		mPipeline(VK_NULL_HANDLE), mPipelineLayout(VK_NULL_HANDLE), mPipelineStates(nullptr)
	{
	}

	void MxVulkanPipeline::clear()
	{
		if (mPipelineStates)
		{
			delete mPipelineStates;
			mPipelineStates = nullptr;
		}
	}

	bool MxVulkanPipeline::setup(const MxVulkanManager * manager)
	{
		if (mIsReady)
			destroy();

		mManager = manager;
		mPipelineStates = new PipelineStates;
		if (!mPipelineStates)
		{
			mManager = nullptr;
			mIsReady = false;
			return false;
		}

		mPipelineStates->vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		mPipelineStates->inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mPipelineStates->rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mPipelineStates->multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mPipelineStates->depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mPipelineStates->colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

		mIsReady = true;
		return true;
	}

	void MxVulkanPipeline::setTargetRenderPass(const VkRenderPass renderPass, const uint32_t subpassIndex)
	{
		mRenderPass = renderPass;
		mSubpassIndex = subpassIndex;
	}

	void MxVulkanPipeline::addShader(const VkShaderStageFlagBits stage, const VkShaderModule shader)
	{
		static char pName[] = "main";
		VkPipelineShaderStageCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.stage = stage;
		createInfo.module = shader;
		createInfo.pName = pName;

		mPipelineStates->shaders.push_back(std::move(createInfo));
	}

	void MxVulkanPipeline::setVertexInput(const std::vector<VkVertexInputBindingDescription>& bindingDescri, const std::vector<VkVertexInputAttributeDescription>& attriDescri)
	{
		mPipelineStates->vertexInput.pVertexBindingDescriptions = bindingDescri.data();
		mPipelineStates->vertexInput.vertexBindingDescriptionCount = bindingDescri.size();
		mPipelineStates->vertexInput.pVertexAttributeDescriptions = attriDescri.data();
		mPipelineStates->vertexInput.vertexAttributeDescriptionCount = attriDescri.size();
	}

	void MxVulkanPipeline::setInputAssembly(const VkPrimitiveTopology topology, const bool primitiveRestart)
	{
		mPipelineStates->inputAssembly.topology = topology;
		mPipelineStates->inputAssembly.primitiveRestartEnable = primitiveRestart;
	}

	void MxVulkanPipeline::addViewport(const std::vector<VkViewport>& viewports)
	{
		mPipelineStates->viewports.insert(mPipelineStates->viewports.end(), viewports.begin(), viewports.end());
	}

	void MxVulkanPipeline::addScissor(const std::vector<VkRect2D>& scissors)
	{
		mPipelineStates->scissors.insert(mPipelineStates->scissors.end(), scissors.begin(), scissors.end());
	}

	void MxVulkanPipeline::setRasterization(const VkPolygonMode polygonMode, const VkCullModeFlags cullMode, const VkFrontFace frontFace, const float lineWidth, const bool depthClampEnable, const bool rasterizerDiscardEnable)
	{

		mPipelineStates->rasterization.polygonMode = polygonMode;
		mPipelineStates->rasterization.cullMode = cullMode;
		mPipelineStates->rasterization.frontFace = frontFace;
		mPipelineStates->rasterization.lineWidth = lineWidth;
		mPipelineStates->rasterization.depthClampEnable = depthClampEnable;
		mPipelineStates->rasterization.rasterizerDiscardEnable = rasterizerDiscardEnable;
	}

	void MxVulkanPipeline::setDepthBias(const bool enable, const float constantFactor, const float slopeFactor, const float clamp)
	{
		mPipelineStates->rasterization.depthBiasEnable = enable;
		mPipelineStates->rasterization.depthBiasConstantFactor = constantFactor;
		mPipelineStates->rasterization.depthBiasSlopeFactor = slopeFactor;
		mPipelineStates->rasterization.depthBiasClamp = clamp;
	}

	void MxVulkanPipeline::setMultiSample(const VkSampleCountFlagBits samples, const bool sampleShading, const float minSampleShading, const VkSampleMask * sampleMask, const bool alphaToCoverageEnable, const bool alphaToOneEnable)
	{

		mPipelineStates->multisample.sampleShadingEnable = sampleShading;
		mPipelineStates->multisample.rasterizationSamples = samples;
		mPipelineStates->multisample.minSampleShading = minSampleShading;
		mPipelineStates->multisample.pSampleMask = sampleMask;
		mPipelineStates->multisample.alphaToCoverageEnable = alphaToCoverageEnable;
		mPipelineStates->multisample.alphaToOneEnable = alphaToOneEnable;
	}

	void MxVulkanPipeline::setDepthTest(const bool depthTestEnable, const bool depthWriteEnable, const VkCompareOp depthCompareOp)
	{
		mPipelineStates->depthStencil.depthTestEnable = depthTestEnable;
		mPipelineStates->depthStencil.depthWriteEnable = depthWriteEnable;
		mPipelineStates->depthStencil.depthCompareOp = depthCompareOp;
	}

	void MxVulkanPipeline::setDepthBoundsTest(const bool enable, const float minBounds, const float maxBounds)
	{
		mPipelineStates->depthStencil.depthBoundsTestEnable = enable;
		mPipelineStates->depthStencil.minDepthBounds = minBounds;
		mPipelineStates->depthStencil.maxDepthBounds = maxBounds;
	}

	void MxVulkanPipeline::setStencilTest(const bool enable, const VkStencilOpState & front, const VkStencilOpState & back)
	{
		mPipelineStates->depthStencil.stencilTestEnable = enable;
		mPipelineStates->depthStencil.front = front;
		mPipelineStates->depthStencil.back = back;
	}

	void MxVulkanPipeline::addBlendAttachments(const std::vector<VkPipelineColorBlendAttachmentState>& attachments)
	{
		mPipelineStates->colorBlendAttachments.insert(mPipelineStates->colorBlendAttachments.end(), attachments.begin(), attachments.end());
	}

	void MxVulkanPipeline::setBlend(const bool logicalOpEnable, const VkLogicOp logicOp, const float constantR, const float constantG, const float constantB, const float constantA)
	{
		mPipelineStates->colorBlend.logicOpEnable = logicalOpEnable;
		mPipelineStates->colorBlend.logicOp = logicOp;
		mPipelineStates->colorBlend.pAttachments = mPipelineStates->colorBlendAttachments.data();
		mPipelineStates->colorBlend.attachmentCount = mPipelineStates->colorBlendAttachments.size();
		mPipelineStates->colorBlend.blendConstants[0] = constantR;
		mPipelineStates->colorBlend.blendConstants[1] = constantG;
		mPipelineStates->colorBlend.blendConstants[2] = constantB;
		mPipelineStates->colorBlend.blendConstants[3] = constantA;
	}

	void MxVulkanPipeline::addDynamicState(const std::vector<VkDynamicState>& dynamicStates)
	{
		mPipelineStates->dynamicStates.insert(mPipelineStates->dynamicStates.end(), dynamicStates.begin(), dynamicStates.end());
	}

	void MxVulkanPipeline::addDescriptorSetLayout(const std::vector<VkDescriptorSetLayout>& setLayouts)
	{
		mPipelineStates->descriptorSetLayouts.insert(mPipelineStates->descriptorSetLayouts.end(), setLayouts.begin(), setLayouts.end());
	}

	void MxVulkanPipeline::addPushConstantRanges(const std::vector<VkPushConstantRange> ranges)
	{
		mPipelineStates->pushConstantRanges.insert(mPipelineStates->pushConstantRanges.end(), ranges.begin(), ranges.end());
	}

	bool MxVulkanPipeline::createPipeline()
	{
		VkPipelineLayoutCreateInfo layoutCreateInfo = {};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCreateInfo.pSetLayouts = mPipelineStates->descriptorSetLayouts.data();
		layoutCreateInfo.setLayoutCount = mPipelineStates->descriptorSetLayouts.size();
		layoutCreateInfo.pPushConstantRanges = mPipelineStates->pushConstantRanges.data();
		layoutCreateInfo.pushConstantRangeCount = mPipelineStates->pushConstantRanges.size();

		if (vkCreatePipelineLayout(mManager->getDevice(), &layoutCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create pipeline layout");

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pViewports = mPipelineStates->viewports.data();
		viewportState.viewportCount = mPipelineStates->viewports.size();
		viewportState.pScissors = mPipelineStates->scissors.data();
		viewportState.scissorCount = mPipelineStates->scissors.size();

		mPipelineStates->colorBlend.pAttachments = mPipelineStates->colorBlendAttachments.data();
		mPipelineStates->colorBlend.attachmentCount = mPipelineStates->colorBlendAttachments.size();

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = mPipelineStates->dynamicStates.data();
		dynamicState.dynamicStateCount = mPipelineStates->dynamicStates.size();

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.pStages = mPipelineStates->shaders.data();
		pipelineCreateInfo.stageCount = mPipelineStates->shaders.size();
		pipelineCreateInfo.pVertexInputState = &mPipelineStates->vertexInput;
		pipelineCreateInfo.pInputAssemblyState = &mPipelineStates->inputAssembly;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pRasterizationState = &mPipelineStates->rasterization;
		pipelineCreateInfo.pMultisampleState = &mPipelineStates->multisample;
		pipelineCreateInfo.pDepthStencilState = &mPipelineStates->depthStencil;
		pipelineCreateInfo.pColorBlendState = &mPipelineStates->colorBlend;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.layout = mPipelineLayout;
		pipelineCreateInfo.renderPass = mRenderPass;
		pipelineCreateInfo.subpass = mSubpassIndex;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;	//基础管线（vulkan允许在已经存在的管线上派生新的管线）
		pipelineCreateInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(mManager->getDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline) != VK_SUCCESS)
			throw std::runtime_error("Error : Failed to create graphics pipeline");

		clear();
		return true;
	}

	void MxVulkanPipeline::destroy()
	{
		if (!mIsReady)
			return;

		vkDestroyPipeline(mManager->getDevice(), mPipeline, nullptr);
		vkDestroyPipelineLayout(mManager->getDevice(), mPipelineLayout, nullptr);
		mManager = nullptr;
		mPipeline = VK_NULL_HANDLE;
		mPipelineLayout = VK_NULL_HANDLE;
	}

	MxVulkanPipeline::~MxVulkanPipeline()
	{
		destroy();
	}
}