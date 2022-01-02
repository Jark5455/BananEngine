//
// Created by yashr on 12/5/21.
//

#pragma once

#include "banan_device.h"

#include <string>
#include <vector>

namespace Banan {

    struct PipelineConfigInfo {
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class BananPipeline {
        public:
            BananPipeline(BananDevice &device, const std::string &vertFilepath, const std::string &fragFilePath, const PipelineConfigInfo &configInfo);

            ~BananPipeline();

            BananPipeline(const BananPipeline&) = delete;
            BananPipeline &operator=(const BananPipeline&) = delete;

            void bind(VkCommandBuffer buffer);
            static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

        private:
            static std::vector<char> readFile(const std::string &filepath);
            void createGraphicsPipeline(const std::string &vertFilePath, const std::string &fragFilePath, const PipelineConfigInfo &info);
            void createShaderModule(const std::vector<char>& code, VkShaderModule *shaderModule);

            BananDevice &device;
            VkPipeline pipeline;
            VkShaderModule vertShaderModule;
            VkShaderModule fragShaderModule;
    };
}