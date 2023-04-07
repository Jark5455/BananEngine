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

                alignas(16) glm::mat4 viewMat0{1.f};
                alignas(16) glm::mat4 viewMat1{1.f};
                alignas(16) glm::mat4 viewMat2{1.f};
                alignas(16) glm::mat4 viewMat3{1.f};
                alignas(16) glm::mat4 viewMat4{1.f};
                alignas(16) glm::mat4 viewMat5{1.f};
            };

            PointShadowSystem(BananDevice &device, BananGameObjectManager &manager, std::vector<VkDescriptorSetLayout> layouts);
            ~PointShadowSystem();

            PointShadowSystem(const PointShadowSystem &) = delete;
            PointShadowSystem &operator=(const PointShadowSystem &) = delete;

            void render(BananFrameInfo info);
            void generateMatrices()

        private:
            void createRenderpass();
            void createFramebuffers();

            void createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
            void createPipeline();

            void beginShadowRenderpass(VkCommandBuffer commandBuffer, BananGameObject::id_t index);
            void endShadowRenderpass(VkCommandBuffer commandBuffer, BananGameObject::id_t index);

            BananDevice &bananDevice;
            BananGameObjectManager &bananGameObjectManager;

            VkRenderPass shadowRenderpass;
            VkFormat depthFormat;

            std::unique_ptr<BananPipeline> bananPipeline;
            VkPipelineLayout pipelineLayout;

            std::unordered_map<BananGameObject::id_t, VkFramebuffer> framebuffers;

            // shader read
            std::unordered_map<BananGameObject::id_t, size_t> cubemapalias;
            std::vector<std::shared_ptr<BananCubemap>> cubemaps;

            // no shader read
            std::unordered_map<BananGameObject::id_t, size_t> depthcubemapalias;
            std::vector<std::shared_ptr<BananCubemap>> depthcubemaps;
    };
}