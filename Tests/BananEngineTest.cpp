//
// Created by yashr on 12/4/21.
//

#include "BananEngineTest.h"

#include "Systems/PointLightSystem.h"
#include "Systems/ResolveSystem.h"
#include "Systems/ComputeSystem.h"
#include "Systems/ProcrastinatedRenderSystem.h"
#include "Systems/PointShadowSystem.h"

#include "Constants/AreaTex.h"
#include "Constants/SearchTex.h"

#include "KeyboardMovementController.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <algorithm>
#include <chrono>

#include <banan_logger.h>

namespace Banan{

    BananEngineTest::BananEngineTest() {

        loadGameObjects();

        globalPool = BananDescriptorPool::Builder(bananDevice)
                .setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();

        procrastinatedPool = BananDescriptorPool::Builder(bananDevice)
                .setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, BananSwapChain::MAX_FRAMES_IN_FLIGHT * 3)
                .build();

        resolvePool = BananDescriptorPool::Builder(bananDevice)
                .setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT * 3)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, BananSwapChain::MAX_FRAMES_IN_FLIGHT * 6)
                .build();
    }

    BananEngineTest::~BananEngineTest() = default;

    void BananEngineTest::run() {

        std::vector<std::unique_ptr<BananBuffer>> uboBuffers(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto & uboBuffer : uboBuffers) {
            uboBuffer = std::make_unique<BananBuffer>(bananDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0);
            uboBuffer->map();
        }

        auto globalSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT, 1)
                .build();

        auto procrastinatedSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                .addBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                .addBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                .build();

        auto edgeDetectionSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS, 1)
                .build();

        auto blendWeightSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS, 1)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS, 1)
                .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS, 1)
                .build();

        auto resolveLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS, 1)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS, 1)
                .build();

        // ComputeSystem computeSystem{bananDevice, {globalSetLayout->getDescriptorSetLayout()}};
        PointShadowSystem pointShadowSystem{bananDevice, bananGameObjectManager, {globalSetLayout->getDescriptorSetLayout(), bananGameObjectManager.getGameObjectSetLayout()}};
        PointLightSystem pointLightSystem{bananDevice, bananRenderer.getGeometryRenderPass(), {globalSetLayout->getDescriptorSetLayout(), bananGameObjectManager.getGameObjectSetLayout()}};
        ProcrastinatedRenderSystem procrastinatedRenderSystem{bananDevice, bananRenderer.getGeometryRenderPass(), {globalSetLayout->getDescriptorSetLayout(), bananGameObjectManager.getGameObjectSetLayout(), bananGameObjectManager.getTextureSetLayout()}, {globalSetLayout->getDescriptorSetLayout(), procrastinatedSetLayout->getDescriptorSetLayout()}};
        ResolveSystem resolveSystem{bananDevice, bananRenderer.getEdgeDetectionRenderPass(), bananRenderer.getBlendWeightRenderPass(), bananRenderer.getResolveRenderPass(), {globalSetLayout->getDescriptorSetLayout(), edgeDetectionSetLayout->getDescriptorSetLayout()}, {globalSetLayout->getDescriptorSetLayout(), blendWeightSetLayout->getDescriptorSetLayout()}, {globalSetLayout->getDescriptorSetLayout(), resolveLayout->getDescriptorSetLayout()}};

        BananCamera camera{};

        std::vector<VkDescriptorSet> globalDescriptorSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<VkDescriptorSet> procrastinatedDescriptorSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<VkDescriptorSet> edgeDetectionDescriptorSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<VkDescriptorSet> blendWeightDescriptorSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<VkDescriptorSet> resolveDescriptorSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < BananSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            BananDescriptorWriter writer = BananDescriptorWriter(*globalSetLayout, *globalPool);
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            writer.writeBuffer(0, bufferInfo);
            writer.build(globalDescriptorSets[i]);

            BananDescriptorWriter procrastinatedWriter = BananDescriptorWriter(*procrastinatedSetLayout, *procrastinatedPool);
            auto gbufferInfo = bananRenderer.getGBufferDescriptorInfo();
            auto albedo = gbufferInfo[0];
            auto normal = gbufferInfo[1];
            auto depth = gbufferInfo[2];

            procrastinatedWriter.writeImage(0, albedo);
            procrastinatedWriter.writeImage(1, normal);
            procrastinatedWriter.writeImage(2, depth);
            procrastinatedWriter.build(procrastinatedDescriptorSets[i]);

            BananDescriptorWriter edgeDetectionWriter(*edgeDetectionSetLayout, *resolvePool);
            auto geomInfo = bananRenderer.getGeometryDescriptorInfo();
            edgeDetectionWriter.writeImage(0, geomInfo);
            edgeDetectionWriter.build(edgeDetectionDescriptorSets[i]);

            BananDescriptorWriter blendWeightWriter(*blendWeightSetLayout, *resolvePool);
            auto edgeInfo = bananRenderer.getEdgeDescriptorInfo();
            auto areaTexInfo = areaTex->descriptorInfo();
            auto searchTexInfo = searchTex->descriptorInfo();
            blendWeightWriter.writeImage(0, edgeInfo);
            blendWeightWriter.writeImage(1, areaTexInfo);
            blendWeightWriter.writeImage(2, searchTexInfo);
            blendWeightWriter.build(blendWeightDescriptorSets[i]);

            BananDescriptorWriter resolveWriter(*resolveLayout, *resolvePool);
            auto blendInfo = bananRenderer.getBlendWeightDescriptorInfo();
            resolveWriter.writeImage(0, geomInfo);
            resolveWriter.writeImage(1, blendInfo);
            resolveWriter.build(resolveDescriptorSets[i]);
        }

        auto &viewerObject = bananGameObjectManager.makeVirtualGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while(true)
        {

            SDL_Event event;
            SDL_PollEvent(&event);

            switch(event.type) {
                case SDL_WINDOWEVENT:
                    if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        bananRenderer.recreateSwapChain();

                        procrastinatedPool->resetPool();
                        resolvePool->resetPool();

                        for (int i = 0; i < BananSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
                            BananDescriptorWriter procrastinatedWriter = BananDescriptorWriter(*procrastinatedSetLayout, *procrastinatedPool);
                            auto gbufferInfo = bananRenderer.getGBufferDescriptorInfo();
                            auto albedo = gbufferInfo[0];
                            auto normal = gbufferInfo[1];
                            auto depth = gbufferInfo[2];

                            procrastinatedWriter.writeImage(0, albedo);
                            procrastinatedWriter.writeImage(1, normal);
                            procrastinatedWriter.writeImage(2, depth);
                            procrastinatedWriter.build(procrastinatedDescriptorSets[i]);

                            BananDescriptorWriter edgeDetectionWriter(*edgeDetectionSetLayout, *resolvePool);
                            auto geomInfo = bananRenderer.getGeometryDescriptorInfo();
                            edgeDetectionWriter.writeImage(0, geomInfo);
                            edgeDetectionWriter.build(edgeDetectionDescriptorSets[i]);

                            BananDescriptorWriter blendWeightWriter(*blendWeightSetLayout, *resolvePool);
                            auto edgeInfo = bananRenderer.getEdgeDescriptorInfo();
                            auto areaTexInfo = areaTex->descriptorInfo();
                            auto searchTexInfo = searchTex->descriptorInfo();
                            blendWeightWriter.writeImage(0, edgeInfo);
                            blendWeightWriter.writeImage(1, areaTexInfo);
                            blendWeightWriter.writeImage(2, searchTexInfo);
                            blendWeightWriter.build(blendWeightDescriptorSets[i]);

                            BananDescriptorWriter resolveWriter(*resolveLayout, *resolvePool);
                            auto blendInfo = bananRenderer.getBlendWeightDescriptorInfo();
                            resolveWriter.writeImage(0, geomInfo);
                            resolveWriter.writeImage(1, blendInfo);
                            resolveWriter.build(resolveDescriptorSets[i]);
                        }

                        // computeSystem.reconstructPipeline({globalSetLayout->getDescriptorSetLayout()});
                        pointLightSystem.reconstructPipeline(bananRenderer.getGeometryRenderPass(), {globalSetLayout->getDescriptorSetLayout(), bananGameObjectManager.getGameObjectSetLayout()});
                        procrastinatedRenderSystem.reconstructPipeline(bananRenderer.getGeometryRenderPass(), {globalSetLayout->getDescriptorSetLayout(), bananGameObjectManager.getGameObjectSetLayout(), bananGameObjectManager.getTextureSetLayout()}, {globalSetLayout->getDescriptorSetLayout(), procrastinatedSetLayout->getDescriptorSetLayout()});
                        resolveSystem.reconstructPipelines(bananRenderer.getEdgeDetectionRenderPass(), bananRenderer.getBlendWeightRenderPass(), bananRenderer.getResolveRenderPass(), {globalSetLayout->getDescriptorSetLayout(), edgeDetectionSetLayout->getDescriptorSetLayout()}, {globalSetLayout->getDescriptorSetLayout(), blendWeightSetLayout->getDescriptorSetLayout()}, {globalSetLayout->getDescriptorSetLayout(), resolveLayout->getDescriptorSetLayout()});
                    }

                    continue;

                case SDL_QUIT:
                    vkDeviceWaitIdle(bananDevice.device());
                    return;
            }

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = bananRenderer.getAspectRatio();
            camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.setPerspectiveProjection(glm::radians(90.f), aspect, 0.1f, 100.f);

            if (auto commandBuffer = bananRenderer.beginFrame()) {
                int frameIndex = bananRenderer.getFrameIndex();
                BananFrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], bananGameObjectManager.getTextureDescriptorSet(frameIndex), bananGameObjectManager.getGameObjectDescriptorSet(frameIndex), procrastinatedDescriptorSets[frameIndex], edgeDetectionDescriptorSets[frameIndex], blendWeightDescriptorSets[frameIndex], resolveDescriptorSets[frameIndex], bananGameObjectManager};

                pointShadowSystem.generateMatrices(frameInfo);

                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                ubo.inverseProjection = camera.getInverseProjection();
                ubo.numGameObjects = (int) bananGameObjectManager.getGameObjects().size();
                ubo.numPointLights = (int) bananGameObjectManager.numPointLights();
                ubo.pointLightBaseRef = bananGameObjectManager.getPointLightBaseRef(frameIndex);

                pointLightSystem.update(frameInfo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                bananGameObjectManager.updateBuffers(frameIndex);

                pointShadowSystem.render(frameInfo);

                bananRenderer.beginGeometryRenderPass(commandBuffer);
                procrastinatedRenderSystem.calculateGBuffer(frameInfo);
                procrastinatedRenderSystem.render(frameInfo);
                pointLightSystem.render(frameInfo);
                bananRenderer.endRenderPass(commandBuffer);

                bananRenderer.beginEdgeDetectionRenderPass(commandBuffer);
                resolveSystem.runEdgeDetection(frameInfo);
                bananRenderer.endRenderPass(commandBuffer);

                bananRenderer.beginBlendWeightRenderPass(commandBuffer);
                resolveSystem.calculateBlendWeights(frameInfo);
                bananRenderer.endRenderPass(commandBuffer);

                bananRenderer.beginResolveRenderPass(commandBuffer);
                resolveSystem.resolveImage(frameInfo);
                bananRenderer.endRenderPass(commandBuffer);

                bananRenderer.endFrame();
            }
        }
    }

    void BananEngineTest::loadGameObjects() {
        // SMAA Textures stuff
        BananBuffer areaTexStagingBuffer{bananDevice, 2, AREATEX_SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        areaTexStagingBuffer.map();
        areaTexStagingBuffer.writeToBuffer((void *) &areaTexBytes, AREATEX_SIZE);

        BananBuffer searchTexStagingBuffer{bananDevice, 1, SEARCHTEX_SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        searchTexStagingBuffer.map();
        searchTexStagingBuffer.writeToBuffer((void *) &searchTexBytes, SEARCHTEX_SIZE);

        VkCommandBuffer commandBuffer = bananDevice.beginSingleTimeCommands();

        areaTex = std::make_unique<BananImage>(bananDevice, AREATEX_WIDTH, AREATEX_HEIGHT, 1, VK_FORMAT_R8G8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        areaTex->transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        searchTex = std::make_unique<BananImage>(bananDevice, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        searchTex->transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        bananDevice.endSingleTimeCommands(commandBuffer);
        commandBuffer = bananDevice.beginSingleTimeCommands();

        bananDevice.copyBufferToImage(areaTexStagingBuffer.getBuffer(), areaTex->getImageHandle(), AREATEX_WIDTH, AREATEX_HEIGHT, 1);
        bananDevice.copyBufferToImage(searchTexStagingBuffer.getBuffer(), searchTex->getImageHandle(), SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 1);

        areaTex->transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        searchTex->transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        bananDevice.endSingleTimeCommands(commandBuffer);

        BananGameObject::Builder vaseBuilder{};
        vaseBuilder.modelPath = "banan_assets/ceramic_vase_01_4k.blend";
        vaseBuilder.albedoPath = "banan_assets/textures/ceramic_vase_01_diff_4k.jpg";
        vaseBuilder.normalPath = "banan_assets/textures/ceramic_vase_01_nor_gl_4k.exr";

        auto &vase = bananGameObjectManager.makeGameObject(vaseBuilder);
        vase.transform.translation = {0.f, .5f, 0.f};
        vase.transform.rotation = {-glm::pi<float>() / 2.0f, 0.f, 0.0f};
        vase.transform.scale = {3.f, 3.f, 3.f};

        BananGameObject::Builder floorBuilder{};
        floorBuilder.modelPath = "banan_assets/quad.obj";
        floorBuilder.albedoPath = "banan_assets/textures/Tiles_046_basecolor.jpg";
        floorBuilder.normalPath = "banan_assets/textures/Tiles_046_normal.exr";
        floorBuilder.heightPath = "banan_assets/textures/Tiles_046_height.png";

        auto &floor = bananGameObjectManager.makeGameObject(floorBuilder);
        floor.transform.translation = {0.f, .5f, 0.f};
        floor.transform.rotation = {0.f, glm::pi<float>(), 0.0f};
        floor.transform.scale = {3.f, 3.f, 3.f};

        floor.parallax.heightscale = 0.1;
        floor.parallax.parallaxBias = -0.02f;
        floor.parallax.numLayers = 48.0f;
        floor.parallax.parallaxmode = 1;

        std::vector<glm::vec4> lightColors{
                {1.f, .1f, .1f, 1.f},
                {.1f, .1f, 1.f, 1.f},
                {.1f, 1.f, .1f, 1.f},
                {1.f, 1.f, .1f, 1.f},
                {.1f, 1.f, 1.f, 1.f},
                {1.f, 1.f, 1.f, 1.f}
        };

        for (size_t i = 0; i < lightColors.size(); i++) {
            auto &pointLight = bananGameObjectManager.makePointLight(0.5f);
            pointLight.pointLight->color = lightColors[i];
            auto rotateLight = glm::rotate(glm::mat4(1.f), (static_cast<float>(i) * glm::two_pi<float>()) / static_cast<float>(lightColors.size()), {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
        }

        bananGameObjectManager.createBuffers(1);
        bananGameObjectManager.buildDescriptors();
    }

    std::shared_ptr<BananLogger> BananEngineTest::getLogger() {
        return bananLogger;
    }
}

