#pragma once

#include "../render/Camera.h"
#include "../render/Device.h"
#include "../render/Pipeline.h"
#include "../render/FrameInfo.h"

#include <memory>
#include <vector>

namespace lm {

	class RenderSystem {
	public:
		RenderSystem(
			lmDevice& device,
			VkRenderPass renderPass,
			VkDescriptorSetLayout globalSetLayout);

		~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator = (const RenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		lmDevice& device;

		std::unique_ptr<lmPipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};

} //namespace lm
