//
// Created by yashr on 4/5/23.
//

#include <banan_device.h>
#include <banan_image.h>
#include <banan_game_object.h>
#include <banan_pipeline.h>
#include <banan_frame_info.h>

#include <unordered_map>

#pragma once

namespace Banan {
    class PointShadowSystem {
        public:

            struct ShadowCubemapMatrices {
                alignas(16) glm::mat4 projectionMatrix{1.f};
                alignas(16) glm::mat4 viewMatrices[6];
            };

            PointShadowSystem(BananDevice &device, BananGameObjectManager &manager, std::vector<VkDescriptorSetLayout> layouts);
            ~PointShadowSystem();

            PointShadowSystem(const PointShadowSystem &) = delete;
            PointShadowSystem &operator=(const PointShadowSystem &) = delete;

            void render(BananFrameInfo frameInfo);
            void generateMatrices(BananFrameInfo frameInfo);

        private:
            void createRenderpass();
            void createFramebuffers();

            void createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
            void createPipeline();

            void beginShadowRenderpass(VkCommandBuffer commandBuffer, BananGameObject::id_t index);
            void endShadowRenderpass(VkCommandBuffer commandBuffer);

            void createMatrixBuffers();
            void createDescriptors();

            BananDevice &bananDevice;
            BananGameObjectManager &bananGameObjectManager;

            VkRenderPass shadowRenderpass;
            VkFormat depthFormat;

            std::unique_ptr<BananPipeline> bananPipeline;
            VkPipelineLayout pipelineLayout;

            std::unordered_map<BananGameObject::id_t, VkFramebuffer> framebuffers;

            std::unordered_map<BananGameObject::id_t, size_t> cubemapalias;
            std::vector<std::shared_ptr<BananCubemap>> cubemaps;
            std::vector<std::shared_ptr<BananCubemap>> depthcubemaps;

            std::vector<std::unique_ptr<BananBuffer>> matriceBuffers;

            std::unique_ptr<BananDescriptorPool> shadowPool;
            std::unique_ptr<BananDescriptorSetLayout> shadowMatrixSetLayout;
            std::vector<VkDescriptorSet> shadowMatrixDescriptorSets;
    };
}