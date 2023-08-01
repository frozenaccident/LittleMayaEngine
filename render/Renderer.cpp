#include "../render/Renderer.h"
#include "../core/Logger.h"

#include <memory>
#include <array>

namespace lm {

	// Constructor: Initializes the lmRenderer object with lmWindow and lmDevice references
	lmRenderer::lmRenderer(lm::lmWindow& window, lm::lmDevice& device) : window{ window }, device{ device } {
		// Recreate the swap chain and create command buffers
		recreateSwapChain();
		createCommandBuffers();

		// Log initialization information
		LOG_INFO("Application initialized");
	}

	// Destructor: Frees the command buffers used by the renderer
	lmRenderer::~lmRenderer() { freeCommandBuffers(); }

	// Recreates the swap chain when the window is resized or first initialized
	void lmRenderer::recreateSwapChain() {
		auto extent = window.getExtent();

		// Wait for the window to be resized properly (width and height must be greater than 0)
		while (extent.width == 0 || extent.height == 0) {
			extent = window.getExtent();
			glfwWaitEvents();
		}

		// Wait for the device to finish operations before continuing
		vkDeviceWaitIdle(device.getDevice());

		// Free old resources before creating new ones
		freeCommandBuffers();

		// Create or recreate the swap chain based on its current state
		if (lmSwapChain == nullptr) {
			lmSwapChain = std::make_unique<lm::lmSwapChain>(device, extent);
		}
		else {
			std::shared_ptr<lm::lmSwapChain> oldSwapChain = std::move(lmSwapChain);
			lmSwapChain = std::make_unique<lm::lmSwapChain>(device, extent, oldSwapChain);

			// Compare the swap chain formats to ensure compatibility
			if (!oldSwapChain->compareSwapFormats(*lmSwapChain.get())) {
				LOG_ERROR("Swap chain image or depth format has changed!");
			}
		}

		// Recreate command buffers after the swap chain has been recreated
		createCommandBuffers();
	}

	// Creates the Vulkan command buffers used for rendering
	void lmRenderer::createCommandBuffers() {
		commandBuffers.resize(lmSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandPool = device.getCommandPool();
		allocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		// Allocate the command buffers from the command pool
		if (vkAllocateCommandBuffers(device.getDevice(), &allocateInfo, commandBuffers.data()) != VK_SUCCESS) {
			LOG_FATAL("Failed to allocate command buffers");
		}

		// Log success message
		LOG_INFO("Command buffers created successfully");
	}

	// Frees the Vulkan command buffers used for rendering
	void lmRenderer::freeCommandBuffers() {
		if (commandBuffers.size() > 0) {
			vkFreeCommandBuffers(
				device.getDevice(),
				device.getCommandPool(),
				static_cast<uint32_t>(commandBuffers.size()),
				commandBuffers.data());

			commandBuffers.clear();
		}
	}

	// Begins the rendering process for a new frame and returns the associated Vulkan command buffer
	VkCommandBuffer lmRenderer::beginFrame() {
		assert(!isFrameStarted && "Cannot call beginFrame while already in progress");

		auto result = lmSwapChain->acquireNextImage(&currentImageIndex);

		// If the swap chain is out of date or suboptimal, recreate it and return a null command buffer
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		// If acquiring the image fails, throw a runtime error
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swapchain image");
		}

		isFrameStarted = true;

		// Get the Vulkan command buffer associated with the new frame
		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// Begin recording the command buffer
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			LOG_ERROR("Failed to begin recording command buffer");
		}

		// Return the command buffer to the caller
		return commandBuffer;
	}

	// Ends the rendering process for the current frame and presents the rendered image to the window
	void lmRenderer::endFrame() {
		assert(isFrameStarted && "Cannot call endFrame while frame already in progress");
		auto commandBuffer = getCurrentCommandBuffer();

		// End recording the command buffer
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			LOG_ERROR("Failed to record command buffer");
		}

		// Submit the command buffer for rendering and presentation
		auto result = lmSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

		// If the swap chain is out of date, suboptimal, or the window was resized, recreate the swap chain
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
			window.resetWindowResizedFlag();
			recreateSwapChain();
		}
		// If submitting the command buffer fails, throw a runtime error
		else if (result != VK_SUCCESS) {
			LOG_ERROR("Failed to present swap chain image");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % lmSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	// Begins a new render pass for the current frame and sets up rendering parameters like viewport and scissor
	void lmRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Cannot call beginSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Cannot begin render pass on a command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = lmSwapChain->getRenderPass();
		renderPassInfo.framebuffer = lmSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = lmSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// Begin the render pass in the specified command buffer
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(lmSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(lmSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, lmSwapChain->getSwapChainExtent() };

		// Set the viewport and scissor in the command buffer
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	// Ends the current render pass for the current frame in the specified command buffer
	void lmRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Cannot call endSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Cannot end the render pass on a command buffer from a different frame");

		// End the current render pass in the specified command buffer
		vkCmdEndRenderPass(commandBuffer);
	}

} // namespace lm
