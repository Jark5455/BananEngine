//
// Created by yashr on 12/5/21.
//

#include "banan_pipeline.h"
#include "banan_model.h"

#include <fstream>
#include <stdexcept>

namespace Banan{
    BananPipeline::BananPipeline(BananDevice &device, const std::string &vertFilepath, const std::string &fragFilePath, const PipelineConfigInfo &configInfo) : device{device} {
        createGraphicsPipeline(vertFilepath, fragFilePath, configInfo);
    }

    BananPipeline::~BananPipeline() {
        vkDestroyShaderModule(device.device(), vertShaderModule, nullptr);
        vkDestroyShaderModule(device.device(), fragShaderModule, nullptr);
        vkDestroyPipeline(device.device(), pipeline, nullptr);
    }

    std::vector<char> BananPipeline::readFile(const std::string &filepath) {
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file: " + filepath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    void BananPipeline::createGraphicsPipeline(const std::string &vertFilePath, const std::string &fragFilePath, const PipelineConfigInfo &info) {

        assert(info.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
        assert(info.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no renderPass provided in configInfo");

        auto vertCode = readFile(vertFilePath);
        auto fragCode = readFile(fragFilePath);

        createShaderModule(vertCode, &vertShaderModule);
        createShaderModule(fragCode, &fragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        auto &bindingDescriptions = info.bindingDescriptions;
        auto &attributeDescriptions = info.attributeDescriptions;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType =VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &info.inputAssemblyInfo;
        pipelineInfo.pViewportState = &info.viewportInfo;
        pipelineInfo.pRasterizationState = &info.rasterizationInfo;
        pipelineInfo.pMultisampleState = &info.multisampleInfo;
        pipelineInfo.pColorBlendState = &info.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &info.depthStencilInfo;
        pipelineInfo.pDynamicState = &info.dynamicStateInfo;

        pipelineInfo.layout = info.pipelineLayout;
        pipelineInfo.renderPass = info.renderPass;
        pipelineInfo.subpass = info.subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }

    void BananPipeline::defaultPipelineConfigInfo(PipelineConfigInfo &configInfo) {
        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.pViewports = nullptr;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = nullptr;

        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0f;
        configInfo.multisampleInfo.pSampleMask = nullptr;
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

        configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0f;
        configInfo.colorBlendInfo.blendConstants[1] = 0.0f;
        configInfo.colorBlendInfo.blendConstants[2] = 0.0f;
        configInfo.colorBlendInfo.blendConstants[3] = 0.0f;

        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = 0.0f;
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {};
        configInfo.depthStencilInfo.back = {};

        configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
        configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
        configInfo.dynamicStateInfo.flags = 0;

        configInfo.bindingDescriptions = BananModel::Vertex::getBindingDescriptions();
        configInfo.attributeDescriptions = BananModel::Vertex::getAttributeDescriptions();
    }

    void BananPipeline::alphaBlendingPipelineConfigInfo(PipelineConfigInfo &configInfo) {
        configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }

    void BananPipeline::shadowPipelineConfigInfo(PipelineConfigInfo &configInfo) {
        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
        configInfo.inputAssemblyInfo.flags = 0;

        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        configInfo.rasterizationInfo.flags = 0;

        configInfo.colorBlendAttachment.colorWriteMask = 0xf;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;

        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.flags = 0;

        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.flags = 0;

        configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
        configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
        configInfo.dynamicStateInfo.flags = 0;

        configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
        configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
        configInfo.dynamicStateInfo.flags = 0;
    }

    void BananPipeline::procrastinatedPipelineConfigInfo(Banan::PipelineConfigInfo &configInfo) {
        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.flags = 0;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        configInfo.rasterizationInfo.flags = 0;
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.rasterizationInfo.lineWidth = 1.0f;

        configInfo.colorBlendAttachment.colorWriteMask = 0xf;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;

        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        configInfo.depthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.flags = 0;

        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.flags = 0;
    }

    void BananPipeline::createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module");
        }
    }

    void BananPipeline::bind(VkCommandBuffer buffer) {
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }
}