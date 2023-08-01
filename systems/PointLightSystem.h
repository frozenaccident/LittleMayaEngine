#pragma once

#include "../render/Camera.h"
#include "../render/Device.h"
#include "../render/Pipeline.h"
#include "../render/FrameInfo.h"
#include "../ecs/GameObject.h"

#include <memory>
#include <vector>

namespace lm {

	class PointLightSystem {
	public:
		PointLightSystem(lmDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator = (const PointLightSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		lm::lmDevice& deviceInstance;

		std::unique_ptr<lmPipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};

} //namespace lm
