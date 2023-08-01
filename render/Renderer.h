#pragma once

#include "../core/Window.h"
#include "../render/Device.h"
#include "../render/Swapchain.h"

#include <cassert>
#include <memory>
#include <vector>

namespace lm {

	class lmRenderer {
	public:
		lmRenderer(lmWindow& window, lmDevice& device);
		~lmRenderer();

		lmRenderer(const lmRenderer&) = delete;
		lmRenderer& operator = (const lmRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return lmSwapChain->getRenderPass(); }

		float getAspectRatio() const { return lmSwapChain->extentAspectRatio(); }

		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		lmWindow& window;
		lmDevice& device;
		std::unique_ptr<lmSwapChain> lmSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted = false;
	};

} //namespace lm
