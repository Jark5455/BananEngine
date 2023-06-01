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
                alignas(16) glm::mat4 invProjectionMatrix{1.f};
                alignas(16) glm::mat4 viewMatrices[6];
                alignas(16) glm::mat4 invViewMatrices[6];
            };

            PointShadowSystem(BananDevice &device, BananGameObjectManager &manager, std::vector<VkDescriptorSetLayout> layouts);
            ~PointShadowSystem();

            PointShadowSystem(const PointShadowSystem &) = delete;
            PointShadowSystem &operator=(const PointShadowSystem &) = delete;

            void render(BananFrameInfo frameInfo);
            void generateMatrices(BananFrameInfo frameInfo);

            VkDescriptorSetLayout getShadowMapsDescriptorSetLayout();
            VkDescriptorSet &getShadowMapDescriptorSet(size_t frameIndex);

        private:
            void createDepthPrepass();
            void createQuantizationPass();

            void createDepthPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
            void createDepthPipeline();

            void createQuantPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
            void createQuantPipeline();

            void createShadowMapFilteringPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
            void createShadowMapFilteringPipeline();

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

            std::unique_ptr<BananPipeline> shadowMapFilteringPipeline;
            VkPipelineLayout shadowMapFilteringPipelineLayout;

            std::unordered_map<BananGameObject::id_t, VkFramebuffer> framebuffers;

            std::unordered_map<BananGameObject::id_t, size_t> cubemapalias;
            std::vector<std::shared_ptr<BananImage>> depthFramebufferImages;
            std::vector<std::shared_ptr<BananCubemap>> quantCubemaps;
            std::vector<std::shared_ptr<BananCubemap>> filteredCubemaps;

            std::vector<std::unique_ptr<BananBuffer>> matriceBuffers;

            std::unique_ptr<BananDescriptorPool> shadowPool;

            std::unique_ptr<BananDescriptorSetLayout> shadowUBOSetLayout;
            std::vector<VkDescriptorSet> shadowUBODescriptorSets;

            std::unique_ptr<BananDescriptorSetLayout> shadowDepthSetLayout;
            std::vector<VkDescriptorSet> shadowDepthDescriptorSets;

            std::unique_ptr<BananDescriptorSetLayout> shadowQuantizationSetLayout;
            std::vector<VkDescriptorSet> shadowQuantizationDescriptorSets;

            std::unique_ptr<BananDescriptorSetLayout> shadowBlurSetLayout;
            std::vector<VkDescriptorSet> shadowBlurPass1DescriptorSets;
            std::vector<VkDescriptorSet> shadowBlurPass2DescriptorSets;

            std::unique_ptr<BananDescriptorSetLayout> shadowMapSetLayout;
            std::vector<VkDescriptorSet> shadowMapDescriptorSets;

            PFN_vkCreateRenderPass2KHR vkCreateRenderPass2Khr;
    };
}