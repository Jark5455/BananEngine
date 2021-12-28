//
// Created by yashr on 12/5/21.
//

#include "banan_pipeline.h"

#include <fstream>
#include <stdexcept>

namespace Banan{
    Banan::BananPipeline::BananPipeline(const std::string &vertFilepath, const std::string &fragFilePath, const PipelineConfigInfo &configInfo) {

    }

    std::vector<char> Banan::BananPipeline::readFile(const std::string &filepath) {
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

    void Banan::BananPipeline::createGraphicsPipeline(const std::string &vertFilePath, const std::string &fragFilePath) {

    }

    PipelineConfigInfo BananPipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
        return PipelineConfigInfo();
    }
}