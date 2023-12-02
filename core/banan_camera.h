//
// Created by yashr on 1/6/22.
//

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Banan {
    class BananCamera {
        public:
            void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
            void setPerspectiveProjection(float fovy, float aspect, float near, float far);

            void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
            void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
            void setViewYXZ(glm::vec3 positon, glm::vec3 rotation);

            const glm::mat4& getProjection() const;
            const glm::mat4& getInverseProjection() const;
            const glm::mat4& getView() const;
            const glm::mat4& getInverseView() const;
            const glm::vec3 getPosition() const;
        private:
            glm::mat4 projectionMatrix{1.f};
            glm::mat4 inverseProjectionMatrix{1.f};
            glm::mat4 viewMatrix{1.f};
            glm::mat4 inverseViewMatrix{1.f};
    };
}
