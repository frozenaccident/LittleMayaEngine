#pragma once

#include "../render/Model.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <unordered_map>

namespace lm {

    struct TransformComponent {
    public:
        glm::vec3 translation{};
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        glm::quat rotation{1, 0, 0, 0};

        bool dirty = true;
        glm::mat4 transform;

        glm::mat3 normalMatrix;

        const glm::vec3& getTranslation() const;
        void setTranslation(const glm::vec3& trans);

        const glm::vec3& getScale() const;
        void setScale(const glm::vec3& scl);

        const glm::quat& getRotation() const;
        void setRotation(const glm::quat& rot);

        void rotate(float angle, const glm::vec3& axis);

        glm::mat4 getMatrix();
        glm::mat3 getNormalMatrix();

        void updateIfNeeded();
        void update();
    };

    struct PointLightComponent {
        float lightIntensity = 1.f;
    };

    class lmGameObject {
    public:
        using id_type = unsigned int;
        using Map = std::unordered_map<id_type, lmGameObject>;

        static lmGameObject createGameObject();

        static lmGameObject makePointLight(
            float intensity = 10.f, float radius = 0.1f, glm::vec4 color = glm::vec4(1.f));

        lmGameObject(const lmGameObject&) = delete;
        lmGameObject& operator=(const lmGameObject&) = delete;
        lmGameObject(lmGameObject&&) = default;
        lmGameObject& operator=(lmGameObject&&) = default;

        id_type getID() const;

        void rotate(float angle, const glm::vec3& axis);

        
        glm::vec3 color{};
        TransformComponent transform{};

        // Optional pointer components
        std::shared_ptr<lmModel> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

    private:
        lmGameObject(id_type objectID);

        id_type id;
    };

} // namespace lm
