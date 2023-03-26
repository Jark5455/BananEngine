//
// Created by yashr on 1/2/22.
//

#include "banan_game_object.h"

#include <algorithm>

namespace Banan {
    BananGameObject::BananGameObject(id_t objId, const BananGameObjectManager &manager) : gameObjectManager{manager} {
        id = objId;
    }

    BananGameObject::id_t BananGameObject::getId() {
        return id;
    }

    glm::mat4 TransformComponent::mat4() {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);

        return glm::mat4{
                {
                        scale.x * (c1 * c3 + s1 * s2 * s3),
                        scale.x * (c2 * s3),
                        scale.x * (c1 * s2 * s3 - c3 * s1),
                        0.0f
                },
                {
                        scale.y * (c3 * s1 * s2 - c1 * s3),
                        scale.y * (c2 * c3),
                        scale.y * (c1 * c3 * s2 + s1 * s3),
                        0.0f
                },
                {
                        scale.z * (c2 * s1),
                        scale.z * (-s2),
                        scale.z * (c1 * c2),
                        0.0f
                },
                {
                    translation.x,
                    translation.y,
                    translation.z,
                    1
                }
        };
    }

    glm::mat3 TransformComponent::normalMatrix() {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        const glm::vec3 invScale = 1.0f / scale;

        return glm::mat3{
                {
                        invScale.x * (c1 * c3 + s1 * s2 * s3),
                        invScale.x * (c2 * s3),
                        invScale.x * (c1 * s2 * s3 - c3 * s1)
                },
                {
                        invScale.y * (c3 * s1 * s2 - c1 * s3),
                        invScale.y * (c2 * c3),
                        invScale.y * (c1 * c3 * s2 + s1 * s3)
                },
                {
                        invScale.z * (c2 * s1),
                        invScale.z * (-s2),
                        invScale.z * (c1 * c2)
                }
        };
    }

    BananGameObjectManager::BananGameObjectManager(BananDevice &device) : bananDevice{device} {

    }

    BananGameObject &BananGameObjectManager::makeVirtualGameObject() {
        auto gameObject = BananGameObject{currentId++, *this};
        auto gameObjectId = gameObject.getId();
        gameObjects.emplace(gameObjectId, std::move(gameObject));

        return gameObjects.at(gameObjectId);
    }

    BananGameObject &BananGameObjectManager::makeGameObject(const BananGameObject::Builder &builder) {
        BananGameObject &gameObject = makeVirtualGameObject();

        if (!builder.modelPath.empty()) {
            gameObject.model = loadMesh(builder.modelPath);
        }

        if (!builder.albedoPath.empty()) {
            gameObject.albedoalias = builder.albedoPath;
            loadImage(builder.albedoPath);
        }

        if (!builder.normalPath.empty()) {
            gameObject.normalalias = builder.normalPath;
            loadImage(builder.normalPath);
        }

        if (!builder.heightPath.empty()) {
            gameObject.heightalias = builder.heightPath;
            loadImage(builder.heightPath);
        }

        return gameObject;
    }

    BananGameObject &BananGameObjectManager::makePointLight(float intensity, float radius, glm::vec3 color) {
        auto gameObject = BananGameObject{currentId++, *this};
        auto gameObjectId = gameObject.getId();
        gameObjects.emplace(gameObjectId, std::move(gameObject));

        gameObjects.at(gameObjectId).pointLight = std::make_unique<PointLightComponent>();
        gameObjects.at(gameObjectId).pointLight->color = color;
        gameObjects.at(gameObjectId).pointLight->lightIntensity = intensity;
        gameObjects.at(gameObjectId).transform.scale.x = radius;
        return gameObjects.at(gameObjectId);
    }

    std::shared_ptr<BananImage> BananGameObjectManager::loadImage(const std::string& filepath) {
        if (texturealias.find(filepath) != texturealias.end())
            return textures.at(texturealias.at(filepath));

        auto image = BananImage::makeImageFromFilepath(bananDevice, filepath);
        textures.push_back(image);
        texturealias.emplace(filepath, textures.size() - 1);

        return image;
    }

    std::shared_ptr<BananModel> BananGameObjectManager::loadMesh(const std::string& filepath) {
        if (modelalias.find(filepath) != modelalias.end())
            return models.at(modelalias.at(filepath));

        auto model = BananModel::createModelFromFile(bananDevice, filepath);
        models.push_back(model);
        modelalias.emplace(filepath, textures.size() - 1);

        return model;
    }

    BananGameObject &BananGameObjectManager::duplicateGameObject(id_t index) {
        auto &object = getGameObjectAtIndex(index);

        auto newObject = BananGameObject{currentId++, *this};
        auto newObjectId = newObject.getId();
        gameObjects.emplace(newObjectId, std::move(newObject));

        auto &new_object = gameObjects.at(newObjectId);

        if (object.pointLight != nullptr) {
            new_object.pointLight = std::make_unique<PointLightComponent>();
            new_object.pointLight->color = object.pointLight->color;
            new_object.pointLight->lightIntensity = object.pointLight->lightIntensity;
            new_object.transform.scale.x = object.transform.scale.x;

            return new_object;
        }


        new_object.transform = object.transform;
        new_object.parallax = object.parallax;

        new_object.albedoalias = object.albedoalias;
        new_object.normalalias = object.normalalias;
        new_object.heightalias = object.heightalias;

        new_object.model = object.model;
        return new_object;
    }

    BananGameObject &BananGameObjectManager::getGameObjectAtIndex(id_t index) {
        return gameObjects.at(index);
    }

    size_t BananGameObjectManager::getImageIndexFromAlias(std::string alias) {
        return texturealias.at(alias);
    }

    size_t BananGameObjectManager::getModelIndexFromAlias(std::string alias) {
        return modelalias.at(alias);
    }
}
