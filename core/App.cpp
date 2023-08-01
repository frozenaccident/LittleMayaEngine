/**
 * @file App.cpp
 * @brief This file implements the main application logic for rendering and handling game objects.
 */

#include "App.h"
#include "Logger.h"
#include "KeyboardMovementController.h"
#include "../systems/RenderSystem.h"
#include "../systems/PointLightSystem.h"
#include "../render/Camera.h"
#include "../render/Buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>

#include <memory>
#include <array>
#include <functional>

/// Define maximum frame time as the inverse of 30 fps
constexpr float MAX_FRAME_TIME = 1.0f / 30.0f;

namespace lm {
	
	App::App() : globalPool(lmDescriptorPool::Builder(lmDevice)
		.setMaxSets(lmSwapChain::MAX_FRAMES_IN_FLIGHT)
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, lmSwapChain::MAX_FRAMES_IN_FLIGHT)
		.build()), assimpImporter(std::make_unique<Assimp::Importer>()) {

		/// Load game objects on application startup
		loadGameObjects();
	}
	
	App::~App() {}
	
	void App::run() {
		LOG_INFO("Running application...");

		// Define vectors for storing uniform buffer objects
		std::vector<std::unique_ptr<lmBuffer>> uboBuffers(lmSwapChain::MAX_FRAMES_IN_FLIGHT);

		// Create a uniform buffer for each frame in flight
		for (auto& uboBuffer : uboBuffers) {
			uboBuffer = std::make_unique<lmBuffer>(
				lmDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			uboBuffer->map();
		}

		// Define descriptor set layout for global uniform buffer
		auto globalSetLayout = lmDescriptorSetLayout::Builder(lmDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		// Allocate and write descriptor sets for each frame in flight
		std::vector<VkDescriptorSet> globalDescriptorSets;
		for (const auto& uboBuffer : uboBuffers) {
			auto bufferInfo = uboBuffer->descriptorInfo();
			VkDescriptorSet globalDescriptorSet;
			lmDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSet);
			globalDescriptorSets.push_back(globalDescriptorSet);
		}

		// Instantiate the render system and point light system
		RenderSystem renderSystem{
			lmDevice,
			lmRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()
		};

		PointLightSystem pointLightSystem{
			lmDevice,
			lmRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()
		};

		// Initialize the camera and viewer object
		lmCamera camera{};
		camera.setViewTarget(glm::vec3(-1.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f)); // The second param corresponds to the center of the model

		auto viewerObject = lmGameObject::createGameObject();
		viewerObject.transform.translation.z = -2.5f;
		KeyboardMovementController cameraController{};

		// Initialize frame timing variables
		float currentTime = static_cast<float>(glfwGetTime());
		float lastTime = currentTime;
		float frameTime = 0.0f;

		// Main loop of the application
		while (!lmWindow.shouldClose()) {
			glfwPollEvents();

			// Calculate time delta between the current frame and the last frame
			currentTime = static_cast<float>(glfwGetTime());
			frameTime = currentTime - lastTime;
			lastTime = currentTime;

			// Cap frame time to max frame time
			frameTime = std::fmin(frameTime, MAX_FRAME_TIME);

			// Handle camera movement input
			cameraController.moveInPlaneXZ(lmWindow.getGLFWwindow(), static_cast<float>(frameTime), viewerObject);
			glm::vec3 eulerRotation = glm::eulerAngles(viewerObject.transform.rotation);
			camera.setViewYXZ(viewerObject.transform.translation, eulerRotation);

			// Update the projection matrix of the camera
			float aspect = lmRenderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

			// Begin a new frame
			if (auto commandBuffer = lmRenderer.beginFrame()) {
				int frameIndex = lmRenderer.getFrameIndex();

				// Prepare frame info
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					gameObjects
				};

				// Update global uniform buffer object
				GlobalUbo ubo{};
				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();
				ubo.inverseView = camera.getInverseView();
				pointLightSystem.update(frameInfo, ubo);
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				// uboBuffers[frameIndex]->flush(); // No need to do it manually since we added VK_MEMORY_PROPERTY_HOST_COHERENT_BIT

				// Render
				lmRenderer.beginSwapChainRenderPass(commandBuffer);

				// Order matters
				renderSystem.renderGameObjects(frameInfo);
				pointLightSystem.render(frameInfo);

				lmRenderer.endSwapChainRenderPass(commandBuffer);
				lmRenderer.endFrame();
			}
		}

		// Wait for the device to finish before exiting the application
		vkDeviceWaitIdle(lmDevice.getDevice());
	}
	
	void App::loadGameObjects() {
		// Load the vase model using Assimp
		const std::string modelPath = std::string(MODEL_DIRECTORY) + "smooth_vase.obj";
		const aiScene* scene = assimpImporter->ReadFile(modelPath, aiProcess_Triangulate | aiProcess_GenNormals);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			LOG_ERROR("Failed to load model: {}", assimpImporter->GetErrorString());
			return;
		}

		// Extract the directory path from the model path
		std::string modelDirectory = modelPath.substr(0, modelPath.find_last_of('/'));

		// Process the scene and create game objects
		processAiNode(scene->mRootNode, scene, modelDirectory, glm::vec3(2.5f), glm::vec3(0.f, 0.5f, 0.f));

		// Load the floor model using Assimp
		const std::string floorModelPath = std::string(MODEL_DIRECTORY) + "floor.obj";
		const aiScene* floorScene = assimpImporter->ReadFile(floorModelPath, aiProcess_Triangulate | aiProcess_GenNormals);
		if (!floorScene || floorScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !floorScene->mRootNode) {
			LOG_ERROR("Failed to load model: {}", assimpImporter->GetErrorString());
			return;
		}

		// Process the scene and create game objects
		processAiNode(floorScene->mRootNode, floorScene, modelDirectory, glm::vec3(1.f), glm::vec3(0.f, 0.5f, 0.f));

		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{ .1f, .1f, 1.f },
			{ .1f, 1.f, .1f },
			{ 1.f, 1.f, .1f },
			{ .1f, 1.f, 1.f },
			{ 1.f, 1.f, 1.f }
		};

		for (int i = 0; i < lightColors.size(); i++) {
			auto pointLight = lmGameObject::makePointLight(0.2f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{ 0.f, -1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getID(), std::move(pointLight));
		}
	}
	
	lmModel::Data App::processAiMesh(aiMesh* mesh, const aiScene* scene, const std::string& modelDirectory) {
		lmModel::Data modelData;

		// Reserve the memory for vertices and indices vectors upfront.
		modelData.vertices.reserve(mesh->mNumVertices);
		modelData.indices.reserve(static_cast<size_t>(mesh->mNumFaces) * 3);

		std::unordered_map<lmModel::Vertex, uint32_t, VertexHash, VertexEqual> uniqueVertices;

		// Process vertices
		for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
			lmModel::Vertex vertex{};

			// Process vertex position
			const aiVector3D& pos = mesh->mVertices[i];
			vertex.position = { pos.x, pos.y, pos.z };

			// Process vertex normal
			if (mesh->HasNormals()) {
				const aiVector3D& normal = mesh->mNormals[i];
				vertex.normal = { normal.x, normal.y, normal.z };
			}

			// Set the default color for each vertex
			vertex.color = { 1.0f, 1.0f, 1.0f }; // Default color: white

			// Check if this vertex is already in our unique vertices
			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(modelData.vertices.size());
				modelData.vertices.push_back(vertex);
			}

			modelData.indices.push_back(uniqueVertices[vertex]);
		}

		return modelData;
	}
	
	void App::processAiNode(aiNode* node, const aiScene* scene, const std::string& modelDirectory, const glm::vec3& scale, const glm::vec3& position) {
		// Process meshes in the current node
		for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			lmModel::Data modelData = processAiMesh(mesh, scene, modelDirectory);
			auto modelInstance = std::make_shared<lmModel>(lmDevice, modelData);

			auto gameObject = lmGameObject::createGameObject();
			gameObject.model = modelInstance;
			gameObject.transform.setScale(scale);
			gameObject.transform.setTranslation(position);
			gameObjects.emplace(gameObject.getID(), std::move(gameObject));
		}

		// Process child nodes recursively
		for (uint32_t i = 0; i < node->mNumChildren; ++i) {
			processAiNode(node->mChildren[i], scene, modelDirectory, scale, position);
		}
	}

} // namespace lm
