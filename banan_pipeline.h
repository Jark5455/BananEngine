//
// Created by yashr on 12/5/21.
//

#pragma once

#include "banan_device.h"

#include <string>
#include <vector>

namespace Banan {

    struct PipelineConfigInfo {};

    class BananPipeline {
        public:
            BananPipeline(const std::string &vertFilepath, const std::string &fragFilePath, const PipelineConfigInfo &configInfo);

            ~BananPipeline();

            BananPipeline(const BananPipeline&) = delete;
            void operator=(const BananPipeline&) = delete;

            static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

        private:
            static std::vector<char> readFile(const std::string &filepath);
            void createGraphicsPipeline(const std::string &vertFilePath, const std::string &fragFilePath);

            BananDevice &device;
            VkPipeline pipeline;
            VkShaderModule vertShaderModule;
    };
}