//
// Created by yashr on 2/22/22.
//

#pragma once

#include "banan_camera.h"

#include <vulkan/vulkan.h>

namespace Banan {
    struct BananFrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        BananCamera &camera;
    };
}