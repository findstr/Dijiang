#include <vector>
#include "vertex.h"
#include "vk_ctx.h"
#include "vk_shader_variables.h"
#include "vk_pipeline.h"

namespace engine {
namespace vulkan {

VkPrimitiveTopology vk_pipeline::primitive_topolgy =
	VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

vk_pipeline *
vk_pipeline::create(VkRenderPass pass, vk_shader *shader, bool ztest)
{
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipelinelayout = VK_NULL_HANDLE;

	VkVertexInputBindingDescription bindingDesc{};
	bindingDesc.binding = 0;
	bindingDesc.stride = render::vertex_type::size() * sizeof(float);
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> attrDescs;
	attrDescs.reserve(shader->vertex_input.attrs.size());
	for (const auto &attr:shader->vertex_input.attrs) {
		auto &ad = attrDescs.emplace_back();
		ad.binding = 0;
		ad.location = attr.location;
		ad.offset = attr.type.offset() * sizeof(float);
		switch (attr.type) {
		case render::vertex_type::POSITION:
			ad.format = VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case render::vertex_type::COLOR:
			ad.format = VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case render::vertex_type::TEXCOORD:
			ad.format = VK_FORMAT_R32G32_SFLOAT;
			break;
                case render::vertex_type::NORMAL:
			ad.format = VK_FORMAT_R32G32B32_SFLOAT;
			break;
                case render::vertex_type::TANGENT:
			ad.format = VK_FORMAT_R32G32B32_SFLOAT;
			break;
                case render::vertex_type::BLENDINDICES:
			ad.format = VK_FORMAT_R32G32B32A32_SINT;
			break;
                case render::vertex_type::BLENDWEIGHT:
			ad.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		default:
			assert(!"unsupport vertex attr type");
			break;
		}
	}

	VkPipelineVertexInputStateCreateInfo vertexInputState = {};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.vertexBindingDescriptionCount = 1;
	vertexInputState.pVertexBindingDescriptions = &bindingDesc;
	vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDescs.size());
	vertexInputState.pVertexAttributeDescriptions = attrDescs.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = primitive_topolgy;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = VK_CTX.swapchain.extent;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.height = (float)VK_CTX.swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkDynamicState dynamicStates[] = {
	#if IS_EDITOR 
		VK_DYNAMIC_STATE_SCISSOR,
	#endif
		VK_DYNAMIC_STATE_VIEWPORT,
	};
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;

	VkDescriptorSetLayout shader_layout = shader->desc_set_layout();
	VkDescriptorSetLayout layouts[] = {shader_layout, VK_CTX.engine_desc_set_layout};
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 2;
	pipelineLayoutInfo.pSetLayouts = layouts;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;
	if (vkCreatePipelineLayout(VK_CTX.device, &pipelineLayoutInfo, nullptr, &pipelinelayout) != VK_SUCCESS)
		return nullptr;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = ztest ? VK_TRUE : VK_FALSE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

	std::vector<VkPipelineShaderStageCreateInfo> shaderstages;
	shaderstages.reserve(5);
	for (auto &iter:shader->modules) {
		auto &ss = shaderstages.emplace_back();
		ss.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ss.stage = iter.stage;
		ss.module = iter.module;
		ss.pName = iter.name.c_str();
		ss.pSpecializationInfo = nullptr;
	}
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderstages.size();
	pipelineInfo.pStages = shaderstages.data();
	pipelineInfo.pVertexInputState = &vertexInputState;
	pipelineInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelinelayout;
	pipelineInfo.renderPass = pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	printf("create pipeline begin\n");
	if (vkCreateGraphicsPipelines(VK_CTX.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		return nullptr;
	printf("create pipeline end\n");
	return new vk_pipeline(pipeline, pipelinelayout);
}

vk_pipeline::~vk_pipeline()
{
	vkDestroyPipeline(VK_CTX.device, handle, nullptr);
	vkDestroyPipelineLayout(VK_CTX.device, layout, nullptr);
}

}}

