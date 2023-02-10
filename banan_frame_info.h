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
        alignas(16) glm::vec4 position{1.f};
        alignas(16) glm::vec4 rotation{1.f};
        alignas(16) glm::vec4 scale{1.f};

        alignas(16) glm::mat4 modelMatrix{1.f};
        alignas(16) glm::mat4 normalMatrix{1.f};

        int hasTexture;
        int hasNormal;

        int hasHeight;
        float heightscale;
        float parallaxBias;
        float numLayers;
        int parallaxmode;

        int isPointLight;
    };

    struct BananFrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        BananCamera &camera;
        VkDescriptorSet globalDescriptorSet;
        VkDescriptorSet textureDescriptorSet;
        VkDescriptorSet normalDescriptorSet;
        VkDescriptorSet heightDescriptorSet;
        VkDescriptorSet procrastinatedDescriptorSet;
        VkDescriptorSet edgeDetectionDescriptorSet;
        VkDescriptorSet blendWeightDescriptorSet;
        VkDescriptorSet resolveDescriptorSet;
        BananGameObject::Map &gameObjects;
    };
}