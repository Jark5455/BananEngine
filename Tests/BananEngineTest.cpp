//
// Created by yashr on 12/4/21.
//

#include "BananEngineTest.h"

#include "Systems/PointLightSystem.h"
#include "Systems/SimpleRenderSystem.h"
#include "Systems/ShadowSystem.h"
#include "Systems/ComputeSystem.h"
#include "Systems/ProcrastinatedRenderSystem.h"

#include "KeyboardMovementController.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <algorithm>
#include <chrono>

#include <banan_logger.h>

#include <sys/resource.h>

namespace Banan{

    BananEngineTest::BananEngineTest() {

        loadGameObjects();

        globalPool = BananDescriptorPool::Builder(bananDevice)
                .setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();

        texturePool = BananDescriptorPool::Builder(bananDevice)
                .setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, BananSwapChain::MAX_FRAMES_IN_FLIGHT * gameObjectsTextureInfo.size())
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();

        normalPool = BananDescriptorPool::Builder(bananDevice)
                .setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, BananSwapChain::MAX_FRAMES_IN_FLIGHT * gameObjectsNormalInfo.size())
                .build();

        heightPool = BananDescriptorPool::Builder(bananDevice)
                .setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, BananSwapChain::MAX_FRAMES_IN_FLIGHT * gameObjectsHeightInfo.size())
                .build();

        procrastinatedPool = BananDescriptorPool::Builder(bananDevice)
                .setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, BananSwapChain::MAX_FRAMES_IN_FLIGHT * 3)
                .build();
    }

    BananEngineTest::~BananEngineTest() = default;

    void BananEngineTest::run() {

        std::vector<std::unique_ptr<BananBuffer>> uboBuffers(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto & uboBuffer : uboBuffers) {
            uboBuffer = std::make_unique<BananBuffer>(bananDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, bananDevice.physicalDeviceProperties().limits.minUniformBufferOffsetAlignment);
            uboBuffer->map();
        }

        std::vector<std::unique_ptr<BananBuffer>> storageBuffers(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto & storageBuffer : storageBuffers) {
            storageBuffer = std::make_unique<BananBuffer>(bananDevice, sizeof(GameObjectData), gameObjects.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, bananDevice.physicalDeviceProperties().limits.minStorageBufferOffsetAlignment);
            storageBuffer->map();
        }

        auto globalSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT, 1)
                .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT, 1)
                .build();

        auto textureSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addFlag(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT)
                .addFlag(VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT)
                .addFlag(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT)
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, gameObjects.size())
                //.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                .build();

        auto normalSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addFlag(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT)
                .addFlag(VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT)
                .addFlag(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT)
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, gameObjects.size())
                .build();

        auto heightMapSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addFlag(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT)
                .addFlag(VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT)
                .addFlag(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT)
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, gameObjects.size())
                .build();

        auto procrastinatedSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                .build();

        SimpleRenderSystem renderSystem{bananDevice, bananRenderer.getSwapChainRenderPass(), {globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout(), normalSetLayout->getDescriptorSetLayout(), heightMapSetLayout->getDescriptorSetLayout()}};
        PointLightSystem pointLightSystem{bananDevice, bananRenderer.getSwapChainRenderPass(), {globalSetLayout->getDescriptorSetLayout()}};
        //ShadowSystem shadowSystem{bananDevice, bananRenderer.getShadowRenderPass(), {globalSetLayout->getDescriptorSetLayout()}};
        ComputeSystem computeSystem{bananDevice, bananRenderer.getSwapChainRenderPass(), {globalSetLayout->getDescriptorSetLayout()}};
        ProcrastinatedRenderSystem procrastinatedRenderSystem{bananDevice, bananRenderer.getGBufferRenderPass(), bananRenderer.getSwapChainRenderPass(), {globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout(), normalSetLayout->getDescriptorSetLayout(), heightMapSetLayout->getDescriptorSetLayout()}, {globalSetLayout->getDescriptorSetLayout(), procrastinatedSetLayout->getDescriptorSetLayout()}};

        BananCamera camera{};

        BananCamera shadowCubeMapCamera{};
        shadowCubeMapCamera.setPerspectiveProjection(glm::radians(-90.f), 1.f, 0.1f, 1024.f);

        std::vector<VkDescriptorSet> globalDescriptorSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<VkDescriptorSet> textureDescriptorSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<VkDescriptorSet> normalDescriptorSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<VkDescriptorSet> heightDescriptorSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<VkDescriptorSet> procrastinatedDescriptorSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < BananSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {

            BananDescriptorWriter writer = BananDescriptorWriter(*globalSetLayout, *globalPool);
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            writer.writeBuffer(0, &bufferInfo);

            auto storageInfo = storageBuffers[i]->descriptorInfo();
            writer.writeBuffer(1, &storageInfo);

            writer.build(globalDescriptorSets[i], std::vector<uint32_t> {});

            BananDescriptorWriter textureWriter = BananDescriptorWriter(*textureSetLayout, *texturePool);
            textureWriter.writeImages(0, gameObjectsTextureInfo);

            textureWriter.build(textureDescriptorSets[i], std::vector<uint32_t> {static_cast<uint32_t>(gameObjectsTextureInfo.size())});

            BananDescriptorWriter normalWriter = BananDescriptorWriter(*normalSetLayout, *normalPool);
            normalWriter.writeImages(0, gameObjectsNormalInfo);

            normalWriter.build(normalDescriptorSets[i], std::vector<uint32_t> {static_cast<uint32_t>(gameObjectsNormalInfo.size())});

            BananDescriptorWriter heightWriter = BananDescriptorWriter(*heightMapSetLayout, *heightPool);
            heightWriter.writeImages(0, gameObjectsHeightInfo);

            heightWriter.build(heightDescriptorSets[i], std::vector<uint32_t> {static_cast<uint32_t>(gameObjectsHeightInfo.size())});

            BananDescriptorWriter procrastinatedWriter = BananDescriptorWriter(*procrastinatedSetLayout, *procrastinatedPool);

            VkDescriptorImageInfo normalInfo = bananRenderer.getGBufferDescriptorInfo()[0];
            VkDescriptorImageInfo albedoInfo = bananRenderer.getGBufferDescriptorInfo()[1];
            VkDescriptorImageInfo depthInfo = bananRenderer.getGBufferDescriptorInfo()[2];

            procrastinatedWriter.writeImage(0, &normalInfo);
            procrastinatedWriter.writeImage(1, &albedoInfo);
            procrastinatedWriter.writeImage(2, &depthInfo);

            procrastinatedWriter.build(procrastinatedDescriptorSets[i], std::vector<uint32_t> {});
        }

        auto viewerObject = BananGameObject::createGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while(!bananWindow.windowShouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(bananWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = bananRenderer.getAspectRatio();
            camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.setPerspectiveProjection(glm::radians(90.f), aspect, 0.1f, 10.f);

            if (auto commandBuffer = bananRenderer.beginFrame()) {
                int frameIndex = bananRenderer.getFrameIndex();
                BananFrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, shadowCubeMapCamera, globalDescriptorSets[frameIndex], textureDescriptorSets[frameIndex], normalDescriptorSets[frameIndex], heightDescriptorSets[frameIndex], procrastinatedDescriptorSets[frameIndex], gameObjects};

                std::vector<GameObjectData> data{};
                data.reserve(gameObjects.size());
                for (auto &kv : gameObjects) {
                    if (kv.second.model != nullptr) {
                        GameObjectData objectData{glm::vec4(kv.second.transform.translation, 0), glm::vec4(kv.second.transform.rotation, 0), glm::vec4(kv.second.transform.scale, 0)};
                        objectData.hasTexture = kv.second.model->isTextureLoaded() ? (int) kv.first : -1;
                        objectData.hasNormal = kv.second.model->isNormalsLoaded() ? (int) kv.first : -1;
                        objectData.isPointLight = 0;

                        if (kv.second.model->isHeightmapLoaded()) {
                            objectData.hasHeight = (int) kv.first;
                            objectData.heightscale = 0.1;
                            objectData.parallaxBias = -0.02f;
                            objectData.numLayers = 48.0f;
                            objectData.parallaxmode = 4;
                        } else {
                            objectData.hasHeight = -1;
                            objectData.heightscale = -1;
                            objectData.parallaxBias = -1;
                            objectData.numLayers = -1;
                            objectData.parallaxmode = -1;
                        }

                        data.push_back(objectData);
                    } else {
                        GameObjectData pointLightData{glm::vec4(kv.second.transform.translation, 0), glm::vec4(kv.second.color, kv.second.pointLight->lightIntensity), glm::vec4(kv.second.transform.scale.x, -1, -1, -1)};
                        pointLightData.hasTexture = -1;
                        pointLightData.hasNormal = -1;
                        pointLightData.isPointLight = 1;
                        pointLightData.hasHeight = -1;
                        pointLightData.heightscale = -1;
                        pointLightData.parallaxBias = -1;
                        pointLightData.numLayers = -1;
                        pointLightData.parallaxmode = -1;

                        data.push_back(pointLightData);
                    }
                }

                std::reverse(data.begin(), data.end());

                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                ubo.shadowProjection = shadowCubeMapCamera.getProjection();
                ubo.heightscale = 0.1;
                ubo.parallaxBias = -0.02f;
                ubo.numLayers = 48.0f;
                ubo.parallaxmode = 4;
                ubo.numGameObjects = gameObjects.size();

                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                storageBuffers[frameIndex]->writeToBuffer(data.data());
                storageBuffers[frameIndex]->flush();

                computeSystem.compute(frameInfo);

                /*for (int i = 0; i < 6; i++) {
                    bananRenderer.beginShadowRenderPass(commandBuffer);
                    shadowSystem.render(frameInfo, i);
                    bananRenderer.endShadowRenderPass(commandBuffer, i);
                }*/

                bananRenderer.beginGBufferRenderPass(commandBuffer);
                procrastinatedRenderSystem.calculateGBuffer(frameInfo);
                bananRenderer.endGBufferRenderPass(commandBuffer);

                bananRenderer.beginSwapChainRenderPass(commandBuffer);
                procrastinatedRenderSystem.render(frameInfo);
                pointLightSystem.render(frameInfo);
                bananRenderer.endSwapChainRenderPass(commandBuffer);

                bananRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(bananDevice.device());
    }

    void BananEngineTest::loadGameObjects() {
        BananModel::Builder vaseBuilder{};
        vaseBuilder.loadModel("banan_assets/ceramic_vase_01_4k.blend");
        vaseBuilder.loadTexture("banan_assets/textures/ceramic_vase_01_diff_4k.jpg");
        vaseBuilder.loadNormals("banan_assets/textures/ceramic_vase_01_nor_gl_4k.exr");

        std::shared_ptr<BananModel> vaseModel = std::make_shared<BananModel>(bananDevice, vaseBuilder);
        auto vase = BananGameObject::createGameObject();
        vase.model = vaseModel;
        vase.transform.translation = {0.f, .5f, 1.f};
        vase.transform.rotation = {-glm::pi<float>() / 2.0f, 0.f, 0.0f};
        vase.transform.scale = {3.f, 3.f, 3.f};
        vase.transform.id = (int) vase.getId();
        gameObjects.emplace(vase.getId(), std::move(vase));

        /*BananModel::Builder otherfloorBuilder{};
        otherfloorBuilder.loadModel("banan_assets/obamium.blend");
        otherfloorBuilder.loadTexture("banan_assets/textures/base.png");

        std::shared_ptr<BananModel> otherfloorModel = std::make_shared<BananModel>(bananDevice, otherfloorBuilder);
        auto otherfloor = BananGameObject::createGameObject();
        otherfloor.model = otherfloorModel;
        otherfloor.transform.translation = {0.f, .3f, 0.f};
        otherfloor.transform.rotation = {glm::pi<float>() / 2.0f, 0.f, 0.f};
        otherfloor.transform.scale = {1.f, 1.f, 1.f};
        otherfloor.transform.id = (int) otherfloor.getId();
        gameObjects.emplace(otherfloor.getId(), std::move(otherfloor));*/

        BananModel::Builder floorBuilder{};
        floorBuilder.loadModel("banan_assets/quad.obj");
        floorBuilder.loadTexture("banan_assets/textures/Tiles_046_basecolor.jpg");
        floorBuilder.loadNormals("banan_assets/textures/Tiles_046_normal.exr");
        floorBuilder.loadHeightMap("banan_assets/textures/Tiles_046_height.png");

        std::shared_ptr<BananModel> floorModel = std::make_shared<BananModel>(bananDevice, floorBuilder);
        auto floor = BananGameObject::createGameObject();
        floor.model = floorModel;
        floor.transform.translation = {0.f, .5f, 2.f};
        floor.transform.rotation = {0.f, 0.f, 0.f};
        floor.transform.scale = {3.f, 3.f, 3.f};
        floor.transform.id = (int) floor.getId();
        gameObjects.emplace(floor.getId(), std::move(floor));

        std::vector<glm::vec3> lightColors{
                {1.f, .1f, .1f},
                {.1f, .1f, 1.f},
                {.1f, 1.f, .1f},
                {1.f, 1.f, .1f},
                {.1f, 1.f, 1.f},
                {1.f, 1.f, 1.f}
        };

        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = BananGameObject::makePointLight(0.5f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(glm::mat4(1.f), (static_cast<float>(i) * glm::two_pi<float>()) / static_cast<float>(lightColors.size()), {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            pointLight.transform.id = 0;
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }

        for (auto &kv : gameObjects)
        {
            if (kv.second.model != nullptr) {
                if (kv.second.model->isTextureLoaded()) {
                    gameObjectsTextureInfo.emplace(kv.first, kv.second.model->getDescriptorTextureImageInfo());
                }

                if (kv.second.model->isNormalsLoaded()) {
                    gameObjectsNormalInfo.emplace(kv.first, kv.second.model->getDescriptorNormalImageInfo());
                }

                if (kv.second.model->isHeightmapLoaded()) {
                    gameObjectsHeightInfo.emplace(kv.first, kv.second.model->getDescriptorHeightMapInfo());
                }
            }
        }
    }

    std::shared_ptr<BananLogger> BananEngineTest::getLogger() {
        return bananLogger;
    }
}

