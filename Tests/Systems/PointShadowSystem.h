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
                alignas(16) glm::mat4 invViewMatrices[6];
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

            void createDepthPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
            void createDepthPipeline();

            void createQuantPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
            void createQuantPipeline();

            void beginShadowRenderpass(VkCommandBuffer commandBuffer, BananGameObject::id_t index);
            void endShadowRenderpass(VkCommandBuffer commandBuffer);

            void createMatrixBuffers();
            void createDescriptors();

            BananDevice &bananDevice;
            BananGameObjectManager &bananGameObjectManager;

            VkRenderPass shadowRenderpass;

            std::unique_ptr<BananPipeline> depthPipeline;
            VkPipelineLayout depthPipelineLayout;

            std::unique_ptr<BananPipeline> quantPipeline;
            VkPipelineLayout quantPipelineLayout;

            std::unordered_map<BananGameObject::id_t, VkFramebuffer> framebuffers;

            std::unordered_map<BananGameObject::id_t, size_t> cubemapalias;
            std::vector<std::shared_ptr<BananImageArray>> depthFramebufferImages;
            std::vector<std::shared_ptr<BananImageArray>> colorFrameBufferImages;
            std::vector<std::shared_ptr<BananCubemap>> quantCubemaps;

            std::vector<std::unique_ptr<BananBuffer>> matriceBuffers;

            std::unique_ptr<BananDescriptorPool> shadowPool;
            std::unique_ptr<BananDescriptorPool> quantPool;

            std::unique_ptr<BananDescriptorSetLayout> shadowMatrixSetLayout;
            std::vector<VkDescriptorSet> shadowMatrixDescriptorSets;

            std::unique_ptr<BananDescriptorSetLayout> quantizationSetLayout;
            std::unordered_map<BananGameObject::id_t, std::vector<VkDescriptorSet>> quantizationDescriptorSets;
    };
}