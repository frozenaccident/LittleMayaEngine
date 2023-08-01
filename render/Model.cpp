#include "Model.h"
#include "Device.h"
#include "Buffer.h"
#include "../core/Logger.h"

#include <cassert>
#include <cstring>

namespace lm {

    /**
     * Constructor for the lmModel class.
     *
     * @param device The Vulkan device used for creating the model.
     * @param data The model data containing vertices and indices.
     */
    lmModel::lmModel(lmDevice& device, const lmModel::Data& data) : deviceInstance{ device } {
        createAttributeBuffers(data.vertices);
        createIndexBuffer(data.indices);
    }

    /**
     * Destructor for the lmModel class.
     */
    lmModel::~lmModel() {}

    /**
     * Create vertex attribute buffers for the model.
     *
     * @param vertices The vector containing the vertex data.
     */
    void lmModel::createAttributeBuffers(const std::vector<Vertex>& vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");

        // Extract position, color, normal, and UV data from vertices
        std::vector<glm::vec3> positions(vertexCount);
        std::vector<glm::vec3> colors(vertexCount);
        std::vector<glm::vec3> normals(vertexCount);
        std::vector<glm::vec2> uvs(vertexCount);

        for (size_t i = 0; i < vertexCount; i++) {
            positions[i] = vertices[i].position;
            colors[i] = vertices[i].color;
            normals[i] = vertices[i].normal;
            uvs[i] = vertices[i].uv;
        }

        VkDeviceSize bufferSize = sizeof(glm::vec3) * vertexCount;

        // Create a single staging buffer for transferring data to GPU
        lmBuffer stagingBuffer{
            deviceInstance,
            bufferSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        stagingBuffer.map();

        // Position Buffer
        positionBuffer = std::make_unique<lmBuffer>(
            deviceInstance,
            bufferSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        stagingBuffer.writeToBuffer(positions.data(), bufferSize);
        deviceInstance.copyBuffer(stagingBuffer.getBuffer(), positionBuffer->getBuffer(), bufferSize);

        // Color Buffer
        colorBuffer = std::make_unique<lmBuffer>(
            deviceInstance,
            bufferSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        stagingBuffer.writeToBuffer(colors.data(), bufferSize);
        deviceInstance.copyBuffer(stagingBuffer.getBuffer(), colorBuffer->getBuffer(), bufferSize);

        // Normal Buffer
        normalBuffer = std::make_unique<lmBuffer>(
            deviceInstance,
            bufferSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        stagingBuffer.writeToBuffer(normals.data(), bufferSize);
        deviceInstance.copyBuffer(stagingBuffer.getBuffer(), normalBuffer->getBuffer(), bufferSize);

        // UV Buffer
        uvBuffer = std::make_unique<lmBuffer>(
            deviceInstance,
            sizeof(glm::vec2) * vertexCount,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        stagingBuffer.writeToBuffer(uvs.data(), sizeof(glm::vec2) * vertexCount);
        deviceInstance.copyBuffer(stagingBuffer.getBuffer(), uvBuffer->getBuffer(), sizeof(glm::vec2) * vertexCount);
    }

    /**
     * Create the index buffer for the model.
     *
     * @param indices The vector containing the index data.
     */
    void lmModel::createIndexBuffer(const std::vector<uint32_t>& indices) {
        if (indices.empty()) {
            hasIndexBuffer = false;
            return;
        }

        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = true;

        VkDeviceSize indexBufferSize = sizeof(indices[0]) * indexCount;

        // Create a staging buffer for transferring index data to GPU
        lmBuffer indexStagingBuffer{
            deviceInstance,
            indexBufferSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        indexStagingBuffer.map();
        indexStagingBuffer.writeToBuffer(indices.data(), indexBufferSize);

        // Create the index buffer on the GPU
        indexBuffer = std::make_unique<lmBuffer>(
            deviceInstance,
            indexBufferSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // Copy index data to the GPU index buffer
        deviceInstance.copyBuffer(indexStagingBuffer.getBuffer(), indexBuffer->getBuffer(), indexBufferSize);
    }

    /**
     * Draw the model using the given command buffer.
     *
     * @param commandBuffer The Vulkan command buffer used for drawing.
     */
    void lmModel::draw(VkCommandBuffer commandBuffer) {
        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        }
        else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }

    /**
     * Bind the model's attribute buffers and index buffer to the given command buffer.
     *
     * @param commandBuffer The Vulkan command buffer used for binding.
     */
    void lmModel::bind(VkCommandBuffer commandBuffer) {
        VkDeviceSize offsets[] = { 0 };

        if (positionBuffer) {
            VkBuffer positionBuffers[] = { positionBuffer->getBuffer() };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, positionBuffers, offsets);
        }

        if (colorBuffer) {
            VkBuffer colorBuffers[] = { colorBuffer->getBuffer() };
            vkCmdBindVertexBuffers(commandBuffer, 1, 1, colorBuffers, offsets);
        }

        if (normalBuffer) {
            VkBuffer normalBuffers[] = { normalBuffer->getBuffer() };
            vkCmdBindVertexBuffers(commandBuffer, 2, 1, normalBuffers, offsets);
        }

        if (uvBuffer) {
            VkBuffer uvBuffers[] = { uvBuffer->getBuffer() };
            vkCmdBindVertexBuffers(commandBuffer, 3, 1, uvBuffers, offsets);
        }

        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    /**
     * Get the vertex binding descriptions for the model.
     *
     * @return A vector of VkVertexInputBindingDescription for the model's vertex attributes.
     */
    std::vector<VkVertexInputBindingDescription> lmModel::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(4);

        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(glm::vec3);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        bindingDescriptions[1].binding = 1;
        bindingDescriptions[1].stride = sizeof(glm::vec3);
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        bindingDescriptions[2].binding = 2;
        bindingDescriptions[2].stride = sizeof(glm::vec3);
        bindingDescriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        bindingDescriptions[3].binding = 3;
        bindingDescriptions[3].stride = sizeof(glm::vec2);
        bindingDescriptions[3].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

    /**
     * Get the vertex attribute descriptions for the model.
     *
     * @return A vector of VkVertexInputAttributeDescription for the model's vertex attributes.
     */
    std::vector<VkVertexInputAttributeDescription> lmModel::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = 0;

        attributeDescriptions[1].binding = 1;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = 0;

        attributeDescriptions[2].binding = 2;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = 0;

        attributeDescriptions[3].binding = 3;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = 0;

        return attributeDescriptions;
    }

}  // namespace lm
