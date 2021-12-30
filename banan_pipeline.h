//
// Created by yashr on 12/5/21.
//

#pragma once

#include "banan_device.h"

#include <string>
#include <vector>

namespace Banan {

    struct PipelineConfigInfo {
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class BananPipeline {
        public:
            BananPipeline(BananDevice &device, const std::string &vertFilepath, const std::string &fragFilePath, const PipelineConfigInfo &configInfo);

            ~BananPipeline();

            BananPipeline(const BananPipeline&) = delete;
            void operator=(const BananPipeline&) = delete;

            static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

        private:
            static std::vector<char> readFile(const std::string &filepath);
            void createGraphicsPipeline(const std::string &vertFilePath, const std::string &fragFilePath, PipelineConfigInfo info);
            void createShaderModule(const std::vector<char>& code, VkShaderModule *shaderModule);

            BananDevice &device;
            VkPipeline pipeline;
            VkShaderModule vertShaderModule;
            VkShaderModule fragShaderModule;
    };
}