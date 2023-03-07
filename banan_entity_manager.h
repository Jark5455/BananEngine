//
// Created by yashr on 3/5/23.
//

#pragma once

#include <map>

#include "banan_descriptor.h"
#include "banan_camera.h"
#include "banan_game_object.h"
#include "banan_image.h"
#include "banan_frame_info.h"

namespace Banan {
    class BananEntityManager{
        public:

            BananEntityManager();
            ~BananEntityManager();

            BananEntityManager(const BananEntityManager &) = delete;
            BananEntityManager &operator=(const BananEntityManager &) = delete;

            void constructGameObject(BananModel::Builder);
            void duplicateInstance(BananGameObject::id_t gameObjectId);
            void generateSSBOs();

            std::vector<GameObjectData> getGameObjectData();
            std::vector<PointLightData> getPointLightData();
        private:

            void loadHDRImage();
            void loadImage();
            void loadMesh();

            std::unordered_map<uint32_t, BananGameObject> gameObjects;
            std::unordered_map<uint32_t, BananGameObject> pointLights;
            std::unordered_map<uint32_t, BananCamera> cameras;

            std::unordered_map<uint32_t, std::shared_ptr<BananImage>> images;
            std::unordered_map<uint32_t, std::shared_ptr<BananModel>> meshes;
    };
}
