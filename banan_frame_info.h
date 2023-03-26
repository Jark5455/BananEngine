//
// Created by yashr on 2/22/22.
//

#pragma once

#include "banan_camera.h"
#include "banan_game_object.h"

#include <vulkan/vulkan.h>

namespace Banan {
    struct GlobalUbo {
        alignas(16) glm::mat4 projection{1.f};
        alignas(16) glm::mat4 inverseProjection{1.f};
        alignas(16) glm::mat4 view{1.f};
        alignas(16) glm::mat4 inverseView{1.f};
        alignas(16) glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.25f};
        int numGameObjects;
    };

    struct GameObjectData {
        alignas(16) glm::vec4 position;
        alignas(16) glm::vec4 rotation;
        alignas(16) glm::vec4 scale;

        alignas(16) glm::mat4 modelMatrix;
        alignas(16) glm::mat4 normalMatrix;

        int hasTexture;
        int hasNormal;
        int hasHeight;

        float heightscale;
        float parallaxBias;
        float numLayers;
        int parallaxmode;

        int isPointLight;
    };

    struct PointLightData {
        alignas(16) glm::vec3 position{1.f};
        alignas(16) glm::vec3 color{1.f};
        int intensity;
    };

    struct BananFrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        BananCamera &camera;
        VkDescriptorSet globalDescriptorSet;
        VkDescriptorSet textureDescriptorSet;
        VkDescriptorSet procrastinatedDescriptorSet;
        VkDescriptorSet edgeDetectionDescriptorSet;
        VkDescriptorSet blendWeightDescriptorSet;
        VkDescriptorSet resolveDescriptorSet;
        BananGameObjectManager &gameObjectManager;
    };
}