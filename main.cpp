// LittleMayaEngine.cpp : Defines the entry point for the application.

#include "core/Logger.h"
#include "core/App.h"

#include <cstdlib>
#include <iostream>

int main()
{
    Logger::init();

    try {

        if (!glfwInit()) {
            LOG_FATAL("Failed to initialize GLFW");
            return EXIT_FAILURE;
        }

        lm::App app{};

        app.run();

        LOG_INFO("Cleaning up GLFW...");
        glfwTerminate();
    }
    catch (const std::exception& e) {
        LOG_FATAL(e.what());
        Logger::getLogger()->flush();
        return EXIT_FAILURE;
    }

    LOG_INFO("Application terminated successfully");
    Logger::getLogger()->flush();
    return EXIT_SUCCESS;
}
