//
// Created by yashr on 4/5/23.
//

#include <banan_device.h>
#include <banan_image.h>
#include <banan_game_object.h>
#include <banan_pipeline.h>

#include <unordered_map>

#pragma once

namespace Banan {
    class PointShadowSystem {
        public:
            PointShadowSystem(BananDevice &device, BananGameObjectManager &manager);
            ~PointShadowSystem();

            PointShadowSystem(const PointShadowSystem &) = delete;
            PointShadowSystem &operator=(const PointShadowSystem &) = delete;

            void reconstructPipeline();
        private:
            void createRenderpass();
            void createFramebuffers();

            void createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
            void createPipeline();

            BananDevice &bananDevice;
            BananGameObjectManager &bananGameObjectManager;

            VkRenderPass shadowRenderpass;
            VkFormat depthFormat;

            std::unique_ptr<BananPipeline> bananPipeline;
            VkPipelineLayout pipelineLayout;

            std::unordered_map<BananGameObject::id_t, VkFramebuffer> framebuffers;
            std::unordered_map<BananGameObject::id_t, std::shared_ptr<BananCubemap>> cubemaps;
            std::unordered_map<BananGameObject::id_t, std::shared_ptr<BananCubemap>> depthcubemaps;
    };
}