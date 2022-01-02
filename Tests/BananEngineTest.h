//
// Created by yashr on 12/4/21.
//
#pragma once

#include "../banan_window.h"
#include "../banan_pipeline.h"
#include "../banan_swap_chain.h"
#include "../banan_device.h"
#include "../banan_model.h"

#include <memory>
#include <vector>

int main();
namespace Banan{
    class BananEngineTest {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        BananEngineTest(const BananEngineTest &) = delete;
        BananEngineTest &operator=(const BananEngineTest &) = delete;

        BananEngineTest();
        ~BananEngineTest();

        void run();
    private:
        void loadModels();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);

        BananWindow bananWindow{WIDTH, HEIGHT};
        BananDevice bananDevice{bananWindow};
        std::unique_ptr<BananSwapChain> bananSwapChain;
        std::unique_ptr<BananPipeline> bananPipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::unique_ptr<BananModel> bananModel;
    };
}
