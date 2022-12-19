//
// Created by yashr on 2/22/22.
//

#pragma once

#include "banan_camera.h"
#include "banan_game_object.h"

#include <vulkan/vulkan.h>

namespace Banan {

    #define MAX_LIGHTS 10
    struct PointLight{
        glm::vec4 positon{};
        glm::vec4 color{};
    };

    struct GlobalUbo {
        glm::mat4 projection{1.f};
        glm::mat4 shadowProjection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseView{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.25f};
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
        float heightscale;
        float parallaxBias;
        float numLayers;
        int parallaxmode;
    };

    struct GameObjectData {
        glm::vec3 position{};
        glm::vec3 rotation{};
        glm::vec3 scale{};

        glm::mat4 modelMatrix{};
        glm::mat4 normalMatrix{};

        int hasTexture;
        int hasNormal;

        int hasHeight;
        float heightscale;
        float parallaxBias;
        float numLayers;
        int parallaxmode;
    };

    struct BananFrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        BananCamera &camera;
        BananCamera &shadowCubeMapCamera;
        VkDescriptorSet globalDescriptorSet;
        VkDescriptorSet textureDescriptorSet;
        VkDescriptorSet normalDescriptorSet;
        VkDescriptorSet heightDescriptorSet;
        BananGameObject::Map &gameObjects;
        uint32_t storageAlignmentSize;
    };
}