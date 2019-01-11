#include"MxVulkan.h"
#include"MxWindow.h"

#include<glm/glm.hpp>
#include<array>
#include<vector>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;

	//����Vertex���ڵĶ����(VertexInputBinding)����
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0; //�������ݵİ󶨵�
		bindingDescription.stride = sizeof(Vertex); //����Vertex֮��ļ��
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	//���ض���������ÿ�� ���Ե����� 
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescription = {};
		attributeDescription[0].binding = 0; //�������ݵİ󶨵�
		attributeDescription[0].location = 0; //��vertex shader�е�location
		attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT; //���Ե����ݸ�ʽ
		attributeDescription[0].offset = offsetof(Vertex, pos); //���������һ��Vertex��ʼλ�õı�����

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, color);

		return attributeDescription;
	}
};

const std::vector<Vertex> vertices =
{
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
	{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	{{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
};

struct UniformBufferObj
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class TestDemo
{
private:
	Mixel::MxWindow* mWindow;
	Mixel::MxVkManager* mManager;
	Mixel::MxVkDebug* mDebug;
	Mixel::MxVkShaderHelper* mShaderHelper;
	Mixel::MxVkSwapchain* mSwapchain;
	Mixel::MxVkRenderPass* mRenderPass;
	Mixel::MxVkDescriptorSetLayout* mDescriptorSetLayout;
	Mixel::MxVkDescriptorPool* mDescriptorPool;
	std::vector<VkDescriptorSet> mDescriptorSets;
	Mixel::MxVkPipeline* mPipeline;
	Mixel::MxVkCommandPool* mCommandPool;
	std::vector<Mixel::MxVkCommandPool::CommandBufferIterator> mCommandBuffers;
	std::vector<Mixel::MxVkBuffer*> mUniformBuffers;
	std::vector<Mixel::MxVkFramebuffer*> mFramebuffers;

	VkSampleCountFlagBits mSampleCount;
	VkViewport mViewport;
	VkRect2D mScissor;
	Mixel::MxVkImage* mDepthImage;

	Mixel::MxVkBuffer* mVertexBuffer;
public:
	TestDemo();
	~TestDemo() = default;
	bool init();
	void run() {};
	void destroy();
};

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cerr << "Error : Failed to initialize SDL2!" << std::endl;
		return -1;
	}

	TestDemo demo;
	demo.init();
	demo.run();
	demo.destroy();
	return 0;
}

TestDemo::TestDemo()
{
	mWindow = new Mixel::MxWindow;
	mManager = new Mixel::MxVkManager;
	mDebug = new Mixel::MxVkDebug;
	mShaderHelper = new Mixel::MxVkShaderHelper;
	mSwapchain = new Mixel::MxVkSwapchain;
	mRenderPass = new Mixel::MxVkRenderPass;
	mDescriptorPool = new Mixel::MxVkDescriptorPool;
	mDescriptorSetLayout = new Mixel::MxVkDescriptorSetLayout;
	mPipeline = new Mixel::MxVkPipeline;

	mSampleCount = VK_SAMPLE_COUNT_1_BIT;
}

bool TestDemo::init()
{
	mWindow->create("Test Demo", 600, 480);

	auto info = mManager->getEmptyInitInfo();

	info->debugMode = true;
	info->present = true;

	info->instance.validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	info->instance.appInfo.appName = mWindow->getTitle();
	info->instance.appInfo.appVersion = VK_MAKE_VERSION(0, 0, 1);
	info->instance.appInfo.engineName = "Mixel";
	info->instance.appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);

	info->device.physical.type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	info->device.physical.enabledFeatures = { false };
	info->device.physical.extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	info->device.physical.queueFlags = VK_QUEUE_GRAPHICS_BIT;
	info->device.physical.queuePrioriy.graphics = 1.0f;

	info->window = mWindow->getWindow();

	try
	{
		mManager->initialize(*info);

		mDebug->setup(mManager);
		mShaderHelper->setup(mManager);

		//setup debug
		mDebug->setDefaultCallback(Mixel::MxVkDebug::SEVERITY_ALL, Mixel::MxVkDebug::TYPE_ALL);

		//setup swapchain
		auto rect = mWindow->getWindowRect();
		mSwapchain->setup(mManager);
		mSwapchain->createSwapchain({ {VK_FORMAT_B8G8R8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } },
									VK_PRESENT_MODE_MAILBOX_KHR,
									{ static_cast<uint32_t>(rect.width),static_cast<uint32_t>(rect.height) });

		//setup renderpass
		mRenderPass->setup(mManager);
		auto presentAttach = mRenderPass->addColorAttach(mSwapchain->getCurrFormat().format, mSampleCount,
														 VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
														 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		auto presentAttachRef = mRenderPass->addColorAttachRef(presentAttach);

		auto depthAttach = mRenderPass->addDepthStencilAttach(VK_FORMAT_D32_SFLOAT, mSampleCount);
		auto depthAttachRef = mRenderPass->addDepthStencilAttachRef(depthAttach);

		auto subpass = mRenderPass->addSubpass();
		mRenderPass->addSubpassColorRef(subpass, presentAttachRef);
		mRenderPass->addSubpassDepthStencilRef(subpass, depthAttachRef);

		mRenderPass->addDependency(VK_SUBPASS_EXTERNAL, subpass,
								   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
								   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
								   0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
		mRenderPass->createRenderPass();

		//setup descriptor
		mDescriptorSetLayout->setup(mManager);
		mDescriptorSetLayout->addBindings(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
		mDescriptorSetLayout->createDescriptorSetLayout();

		//load shader
		mShaderHelper->setup(mManager);
		auto vertexShader = mShaderHelper->createModule("Shader/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		auto fragShader = mShaderHelper->createModule("Shader/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		//setup graphics pipeline
		mPipeline->setup(mManager);
		mPipeline->setTargetRenderPass(mRenderPass->getRenderPass(), 0);

		mPipeline->addShader(vertexShader->stage, vertexShader->module);
		mPipeline->addShader(fragShader->stage, fragShader->module);

		//setup vertex input state
		//todo create a new class to deal with this
		std::vector<VkVertexInputBindingDescription> inputBinding;
		std::vector<VkVertexInputAttributeDescription> inputAttri;
		inputBinding.push_back(Vertex::getBindingDescription());
		auto attri = Vertex::getAttributeDescription();
		inputAttri.insert(inputAttri.end(), attri.begin(), attri.end());
		mPipeline->setVertexInput(inputBinding, inputAttri);

		//setup input assembly
		mPipeline->setInputAssembly();

		//setup viewport
		mViewport.x = 0;
		mViewport.y = 0;
		mViewport.width = static_cast<float>(mWindow->getWindowRect().width);
		mViewport.height = static_cast<float>(mWindow->getWindowRect().height);
		mViewport.minDepth = 0.0f;
		mViewport.maxDepth = 1.0f;
		mPipeline->addViewport(mViewport);

		mScissor.extent = mSwapchain->getCurrExtent();
		mScissor.offset = { 0,0 };
		mPipeline->addScissor(mScissor);

		//setup rasterization
		mPipeline->setRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		mPipeline->setDepthBias(false);

		//setup multisample
		mPipeline->setMultiSample(mSampleCount);

		//setup depth/stencil test
		mPipeline->setDepthTest(true);
		mPipeline->setDepthBoundsTest(false);
		mPipeline->setStencilTest(false);

		//setup blend
		mPipeline->addDefaultBlendAttachments();

		//setup layout
		mPipeline->addDescriptorSetLayout(mDescriptorSetLayout->getDescriptorSetLayout());
		mPipeline->addPushConstantRanges();

		//create graphics pipeline
		mPipeline->createPipeline();

		//create depth stencil buffer
		mDepthImage = Mixel::MxVkImage::createDepthStencil(mManager, VK_FORMAT_D24_UNORM_S8_UINT,
															   mSwapchain->getCurrExtent(),
															   mSampleCount);

		//setup framebuffer
		std::vector<VkImageView> attachments;
		mFramebuffers.resize(mSwapchain->getImageCount());
		for (uint32_t i = 0; i < mFramebuffers.size(); ++i)
		{
			mFramebuffers[i] = new Mixel::MxVkFramebuffer;
			mFramebuffers[i]->setup(mManager);
			mFramebuffers[i]->setExtent({ static_cast<uint32_t>(rect.width),static_cast<uint32_t>(rect.height) });
			mFramebuffers[i]->setLayers(1);

			attachments = { mSwapchain->getImageViews()[i],mDepthImage->view };
			mFramebuffers[i]->addAttachments(attachments);
		}

		//create vertex buffer
		{
			VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
			//temporary buffer
			mVertexBuffer = Mixel::MxVkBuffer::createBuffer(mManager, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
																VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize);
			Mixel::MxVkBuffer::copyToDeviceBuffer(mManager, mCommandPool, mVertexBuffer, vertices.data());
		}

		//create uniform buffer
		{
			mUniformBuffers.resize(mSwapchain->getImageCount());
			for (size_t i = 0; i < mSwapchain->getImageCount(); ++i)
			{
				mUniformBuffers[i] = Mixel::MxVkBuffer::createBuffer(mManager, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																		 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
																		 sizeof(UniformBufferObj));
			}
		}

		//create descriptor pool
		mDescriptorPool->addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, mSwapchain->getImageCount());
		mDescriptorPool->createDescriptorPool(mSwapchain->getImageCount());

		//allocate descriptor sets
		mDescriptorSets = mDescriptorPool->allocDescriptorSet(*mDescriptorSetLayout, mSwapchain->getImageCount());

		//create commandbuffer
		{
			auto its = mCommandPool->allocCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, mSwapchain->getImageCount());
			mCommandBuffers.assign(its.first, its.second);

			for (size_t i = 0; i < mCommandBuffers.size(); ++i)
			{
				//begin command buffer
				mCommandPool->beginCommandBuffer(mCommandBuffers[i]);

				std::vector<VkClearValue> clearValues(2);
				clearValues[0].color = { 0.0f,0.0f,0.0f,1.0f };
				clearValues[1].depthStencil = { 1.0f,0 };

				//begin render pass
				mRenderPass->beginRenderPass(*mCommandBuffers[i], mFramebuffers[i]->getFramebuffer(), clearValues,
											 mSwapchain->getCurrExtent());

				//bind pipeline
				vkCmdBindPipeline(*mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->getPipeline());

				//bind vertex buffer
				VkBuffer vertexBuffers[] = { mVertexBuffer->buffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(*mCommandBuffers[i], 0, 1, vertexBuffers, offsets);

				//bind descriptor sets
				vkCmdBindDescriptorSets(*mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->getPipelineLayout(),
										0, //��һ�����������ϵ����� 
										1, //������������
										&mDescriptorSets[i], //����������
										0, nullptr);

				//draw
				vkCmdDraw(*mCommandBuffers[i], vertices.size(), 1, 0, 0);

				//end render pass
				mRenderPass->endRenderPass(*mCommandBuffers[i]);

				//end command buffer
				mCommandPool->endCommandBuffer(mCommandBuffers[i]);
			}
		}

	}

	catch (const std::exception& e)
	{
		std::cout << e.what() << std::ends << std::endl;
	}

	return true;
}

void TestDemo::destroy()
{
	for (auto& framebuffer : mFramebuffers)
		MX_FREE_OBJECT(framebuffer);

	MX_FREE_OBJECT(mShaderHelper);
	MX_FREE_OBJECT(mSwapchain);
	MX_FREE_OBJECT(mDescriptorSetLayout);
	MX_FREE_OBJECT(mDescriptorPool);
	MX_FREE_OBJECT(mPipeline);
	MX_FREE_OBJECT(mRenderPass);
	MX_FREE_OBJECT(mDebug);

	vkDestroyImageView(mManager->getDevice(), mDepthImage->view, nullptr);
	vkDestroyImage(mManager->getDevice(), mDepthImage->image, nullptr);
	vkFreeMemory(mManager->getDevice(), mDepthImage->memory, nullptr);

	MX_FREE_OBJECT(mManager);
	MX_FREE_OBJECT(mWindow);
}