/**
* @class TransformComponent
* @brief A class for managing transformations in 3D space.
*/

#include "GameObject.h"

namespace lm {   

     /**
      * @brief Retrieves the translation vector of the TransformComponent.
      * @return The current translation vector.
      */
    const glm::vec3& TransformComponent::getTranslation() const {
        return translation;
    }

    /**
     * @brief Sets the translation vector of the TransformComponent.
     * @param trans The new translation vector.
     */
    void TransformComponent::setTranslation(const glm::vec3& trans) {
        translation = trans;
        dirty = true;
    }

    /**
     * @brief Retrieves the scale vector of the TransformComponent.
     * @return The current scale vector.
     */
    const glm::vec3& TransformComponent::getScale() const {
        return scale;
    }

    /**
     * @brief Sets the scale vector of the TransformComponent.
     * @param scl The new scale vector.
     */
    void TransformComponent::setScale(const glm::vec3& scl) {
        scale = scl;
        dirty = true;
    }

    /**
     * @brief Retrieves the rotation quaternion of the TransformComponent.
     * @return The current rotation quaternion.
     */
    const glm::quat& TransformComponent::getRotation() const {
        return rotation;
    }

    /**
     * @brief Sets the rotation quaternion of the TransformComponent.
     * @param rot The new rotation quaternion.
     */
    void TransformComponent::setRotation(const glm::quat& rot) {
        rotation = rot;
        dirty = true;
    }

    /**
     * @brief Rotates the TransformComponent by a specified angle around a specified axis.
     * @param angle The angle to rotate by.
     * @param axis The axis to rotate around.
     */
    void TransformComponent::rotate(float angle, const glm::vec3& axis) {
        rotation = glm::normalize(glm::angleAxis(glm::radians(angle), glm::normalize(axis)) * rotation);
        dirty = true;
    }

    /**
     * @brief Retrieves the transformation matrix of the TransformComponent.
     * @return The current transformation matrix.
     */
    glm::mat4 TransformComponent::getMatrix() {
        updateIfNeeded();
        return transform;
    }

    /**
     * @brief Retrieves the normal matrix of the TransformComponent.
     * @return The current normal matrix.
     */
    glm::mat3 TransformComponent::getNormalMatrix() {
        updateIfNeeded();
        return normalMatrix;
    }

    /**
     * @brief Updates the TransformComponent if it's marked as dirty.
     */
    void TransformComponent::updateIfNeeded() {
        if (dirty) {
            update();
        }
    }

    /**
     * @brief Updates the transformation and normal matrices of the TransformComponent.
     */
    void TransformComponent::update() {
        transform = glm::mat4(1.0f);
        transform = glm::translate(transform, translation);
        transform = transform * glm::mat4_cast(rotation);
        transform = glm::scale(transform, scale);
        normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
        dirty = false;
    }

    /**
     * @class lmGameObject
     * @brief A class for managing game objects.
     */

     /**
      * @brief Constructor for lmGameObject with a specified object ID.
      * @param objectID The unique ID for the new object.
      */
    lmGameObject::lmGameObject(lmGameObject::id_type objectID) : id(objectID) {}

    /**
     * @brief Retrieves the unique ID of the lmGameObject.
     * @return The unique ID of the object.
     */
    lmGameObject::id_type lmGameObject::getID() const {
        return id;
    }

    /**
     * @brief Rotates the lmGameObject by a specified angle around a specified axis.
     * @param angle The angle to rotate by.
     * @param axis The axis to rotate around.
     */
    void lmGameObject::rotate(float angle, const glm::vec3& axis) {
        transform.rotate(angle, axis);
    }

    /**
     * @brief Creates a new lmGameObject with an automatically generated unique ID.
     * @return The new lmGameObject.
     */
    lmGameObject lmGameObject::createGameObject() {
        static std::atomic<id_type> currentID = 0;
        return lmGameObject{ currentID++ };
    }

    /**
     * @brief Creates a new lmGameObject representing a point light source.
     * @param intensity The intensity of the light source.
     * @param radius The radius of the light source.
     * @param color The color of the light source.
     * @return The new lmGameObject.
     */
    lmGameObject lmGameObject::makePointLight(float intensity, float radius, glm::vec4 color) {
        lmGameObject gameObj = lmGameObject::createGameObject();
        gameObj.color = color;
        gameObj.transform.scale.x = radius;
        gameObj.pointLight = std::make_unique<PointLightComponent>();
        gameObj.pointLight->lightIntensity = intensity;

        return gameObj;
    }

}  // namespace lm
