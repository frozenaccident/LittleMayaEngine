#pragma once

#include "Device.h"
#include "Buffer.h"

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

namespace lm {

    class lmModel {
    public:

        struct Vertex {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec3 normal;
            glm::vec2 uv;
        };

        struct Data {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
        };

        lmModel(lmDevice& device, const lmModel::Data& data);
        ~lmModel();

        lmModel(const lmModel&) = delete;
        lmModel& operator=(const lmModel&) = delete;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createAttributeBuffers(const std::vector<Vertex>& vertices);
        void createIndexBuffer(const std::vector<uint32_t>& indices);

        lmDevice& deviceInstance;
        std::unique_ptr<lmBuffer> positionBuffer;
        std::unique_ptr<lmBuffer> colorBuffer;
        std::unique_ptr<lmBuffer> normalBuffer;
        std::unique_ptr<lmBuffer> uvBuffer;
        std::unique_ptr<lmBuffer> indexBuffer;
        uint32_t vertexCount;
        uint32_t indexCount;
        bool hasIndexBuffer = false;
    };

}  // namespace lm
