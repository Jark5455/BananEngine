//
// Created by yashr on 12/21/22.
//

#include "ProcrastinatedRenderSystem.h"

namespace Banan {
    ProcrastinatedRenderSystem::ProcrastinatedRenderSystem(BananDevice &device, VkRenderPass GBufferRenderPass, VkRenderPass mainRenderPass, std::vector<VkDescriptorSetLayout> layouts, std::vector<VkDescriptorSetLayout> procrastinatedLayouts) : bananDevice{device} {
    }

    ProcrastinatedRenderSystem::~ProcrastinatedRenderSystem() {

    }

    void ProcrastinatedRenderSystem::calculateGBuffer(BananFrameInfo &frameInfo) {

    }

    void ProcrastinatedRenderSystem::render(BananFrameInfo &frameInfo) {

    }
}