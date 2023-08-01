#include "Window.h"

namespace lm {

	lmWindow::lmWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	lmWindow::~lmWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void lmWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void lmWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			LOG_FATAL("Failed to create window surface");
		}
	}

	void lmWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto lmWindow = reinterpret_cast<lm::lmWindow*>(glfwGetWindowUserPointer(window));
		lmWindow->framebufferResized = true;
		lmWindow->width = width;
		lmWindow->height = height;
	}

}// namespace lm