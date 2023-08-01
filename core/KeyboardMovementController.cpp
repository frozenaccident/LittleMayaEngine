#include "KeyboardMovementController.h"

namespace lm {

	void KeyboardMovementController::moveInPlaneXZ(
		GLFWwindow* window, float deltaTime, lmGameObject& gameObject) {

		glm::vec3 rotate{ 0 };

		if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) { rotate.y -= 1.f; }
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) { rotate.y += 1.f; }
		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) { rotate.x -= 1.f; }
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) { rotate.x += 1.f; }

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			// Normalize rotate vector
			rotate = glm::normalize(rotate);

			// Create rotation quaternion based on user input
			glm::quat rotationDelta = glm::angleAxis(glm::length(rotate) * deltaTime * lookSpeed, rotate);

			// Apply rotation
			gameObject.transform.rotation = rotationDelta * gameObject.transform.rotation;
		}

		// Retrieve rotation matrix from current rotation quaternion
		glm::mat3 rotationMatrix = glm::mat3_cast(gameObject.transform.rotation);

		// Set forward, right and up directions based on rotation matrix
		const glm::vec3 forwardDir = rotationMatrix * glm::vec3(0.0f, 0.0f, -1.0f);
		const glm::vec3 rightDir = rotationMatrix * glm::vec3(1.0f, 0.0f, 0.0f);
		const glm::vec3 upDir = rotationMatrix * glm::vec3(0.0f, 1.0f, 0.0f);

		glm::vec3 moveDir{ 0.f };

		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir -= forwardDir;
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir += forwardDir;
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir -= rightDir;
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir += rightDir;
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir -= upDir;
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir += upDir;

		// Here, you would apply moveDir to your game object's position
		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.translation += moveSpeed * deltaTime * glm::normalize(moveDir);
		}
	}

}// namespace lm