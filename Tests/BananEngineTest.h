//
// Created by yashr on 12/4/21.
//
#pragma once

#include "../banan_window.h"
#include "../banan_pipeline.h"

int main();
namespace Banan{
    class BananEngineTest {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        void run();
    private:
        BananWindow bananWindow{WIDTH, HEIGHT};
        BananDevice bananDevice{bananWindow};
        BananPipeline bananPipeline{bananDevice, "shaders/triangle.vert.spv", "shaders/triangle.frag.spv", BananPipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
    };
}
