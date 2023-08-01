#include "../systems/RenderSystem.h"
#include "../core/Logger.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>
#include <array>

namespace lm {

	struct PushConstantData {
		glm::mat4 modelMatrix{1.f};
		glm::mat4 normalMatrix{1.f};
	};

	RenderSystem::RenderSystem(
		lmDevice& device,
		VkRenderPass renderPass,
		VkDescriptorSetLayout globalSetLayout) : device{ device } {
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
	}

	RenderSystem::~RenderSystem() {
		vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
	}

	void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			LOG_FATAL("Failed to create pipeline layout");
		}

		LOG_INFO("Pipeline layout created successfully");
	}

	void RenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		lmPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;

		pipeline = std::make_unique<lmPipeline>(
			device,
			"shaders/shader.vert.spv",
			"shaders/shader.frag.spv",
			pipelineConfig);

		LOG_INFO("Pipeline created successfully");
	}

	void RenderSystem::renderGameObjects(FrameInfo& frameInfo) {
		pipeline->bind(frameInfo.commandBuffer);	

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr);

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;

			if (obj.model == nullptr) continue;

			PushConstantData push{};			
			push.modelMatrix = obj.transform.getMatrix();
			// Ensures that lighting calculations remain correct when non-uniform scaling is applied to a model
			push.normalMatrix = glm::transpose(glm::inverse(glm::mat3(obj.transform.getMatrix())));

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PushConstantData),
				&push);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}

}// namespace lm
