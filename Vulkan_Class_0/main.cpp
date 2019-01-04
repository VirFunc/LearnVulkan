#include"MxVulkan.h"
#include"MxWindow.h"

#include<glm/glm.hpp>
#include<array>
#include<vector>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	//返回Vertex对于的顶点绑定(VertexInputBinding)描述
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0; //顶点数据的绑定点
		bindingDescription.stride = sizeof(Vertex); //两个Vertex之间的间隔
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	//返回顶点数据中每个 属性的描述 
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescription = {};
		attributeDescription[0].binding = 0; //顶点数据的绑定点
		attributeDescription[0].location = 0; //在vertex shader中的location
		attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT; //属性的数据格式
		attributeDescription[0].offset = offsetof(Vertex, pos); //属性相对于一个Vertex起始位置的便宜量

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, color);

		attributeDescription[2].binding = 0;
		attributeDescription[2].location = 2;
		attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[2].offset = offsetof(Vertex, texCoord);

		return attributeDescription;
	}
};

class TestDemo
{
private:
	Mixel::MxWindow* mWindow;
	Mixel::MxVulkanManager* mManager;
	Mixel::MxVulkanDebug* mDebug;
	Mixel::MxVulkanShaderHelper* mShaderHelper;
	Mixel::MxVulkanSwapchain* mSwapchain;
	Mixel::MxVulkanRenderPass* mRenderPass;
	Mixel::MxVulkanDescriptorSetLayout* mDescriptorSetLayout;
	Mixel::MxVulkanDescriptorPool* mDescriptorPool;
	Mixel::MxVulkanPipeline* mPipeline;
	std::vector<Mixel::MxVulkanFramebuffer*> mFramebuffers;

	VkSampleCountFlagBits mSampleCount;
	VkViewport mViewport;
	VkRect2D mScissor;
	Mixel::MxVulkanImage* mDepthImage;
public:
	TestDemo();
	~TestDemo() = default;
	bool init();
	void run() {};
	void destroy() {};
};

int main()
{
	return 0;
}

TestDemo::TestDemo()
{
	mWindow = new Mixel::MxWindow;
	mManager = new Mixel::MxVulkanManager;
	mDebug = new Mixel::MxVulkanDebug;
	mShaderHelper = new Mixel::MxVulkanShaderHelper;
	mSwapchain = new Mixel::MxVulkanSwapchain;
	mRenderPass = new Mixel::MxVulkanRenderPass;
	mDescriptorPool = new Mixel::MxVulkanDescriptorPool;
	mDescriptorSetLayout = new Mixel::MxVulkanDescriptorSetLayout;
	mPipeline = new Mixel::MxVulkanPipeline;

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
		mDebug->setDefaultCallback(Mixel::MxVulkanDebug::SEVERITY_ALL, Mixel::MxVulkanDebug::TYPE_ALL);

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
		mDescriptorSetLayout->addBindings(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
		mDescriptorSetLayout->createDescriptorSetLayout();

		//load shader
		mShaderHelper->setup(mManager);
		auto vertexShader = mShaderHelper->createModule("Shader/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		auto fragShader = mShaderHelper->createModule("Shader/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		//setup graphics pipeline
		mPipeline->setup(mManager);
		mPipeline->addShader(vertexShader->stage, vertexShader->module);
		mPipeline->addShader(fragShader->stage, fragShader->module);

		//setup vertex input state
		//todo create a new class to deal with this
		std::vector<VkVertexInputBindingDescription> inputBinding(1);
		std::vector<VkVertexInputAttributeDescription> inputAttri(1);
		inputBinding.push_back(Vertex::getBindingDescription());
		auto attri = Vertex::getAttributeDescription();
		inputAttri.insert(inputAttri.end(), attri.begin(), attri.end());
		mPipeline->setVertexInput(inputBinding, inputAttri);

		//setup input assembly
		mPipeline->setInputAssembly();

		//setup viewport
		mViewport.x = 0;
		mViewport.y = 0;
		mViewport.width = mWindow->getWindowRect().width;
		mViewport.height = mWindow->getWindowRect().height;
		mViewport.minDepth = 0.0f;
		mViewport.maxDepth = 1.0f;
		mPipeline->addViewport(mViewport);

		mScissor.extent = mSwapchain->getCurrExtent();
		mScissor.offset = { 0,0 };

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
		mDepthImage = Mixel::MxVulkanImage::createDepthStencil(mManager,);

		//setup framebuffer
		std::vector<VkImageView> attachments;
		mFramebuffers.resize(mSwapchain->getImageCount());
		for (uint32_t i = 0; i < mFramebuffers.size(); ++i)
		{
			mFramebuffers[i] = new Mixel::MxVulkanFramebuffer;
			mFramebuffers[i]->setup(mManager);
			mFramebuffers[i]->setExtent({ rect.width,rect.height });
			mFramebuffers[i]->setLayers(1);

			attachments = { mSwapchain->getImageViews()[i],mDepthImage->view };
			mFramebuffers[i]->addAttachments(attachments);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::ends << std::endl;
	}

	return true;
}
