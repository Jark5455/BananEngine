//
// Created by yashr on 12/21/22.
//

#include "ProcrastinatedRenderSystem.h"

namespace Banan {
    ProcrastinatedRenderSystem::ProcrastinatedRenderSystem(BananDevice &device, VkRenderPass GBufferRenderPass, VkRenderPass mainRenderPass, std::vector<VkDescriptorSetLayout> layouts) : bananDevice{device} {
        createGBufferPipelineLayout(GBufferRenderPass);
        createMainRenderTargetPipelineLayout(GBufferRenderPass);

        createGBufferPipeline(layouts);
        createMainRenderTargetPipeline(layouts);
    }
}