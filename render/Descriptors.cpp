#include "Descriptors.h"

#include <cassert>

namespace lm {

    // *********************** Descriptor Set Layout Builder ***********************

    // Add a new descriptor binding to the Descriptor Set Layout Builder.
    lmDescriptorSetLayout::Builder& lmDescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;

        // If the descriptor type is a uniform buffer, ensure it is accessible
        // from both the vertex and the fragment shader stages
        if (descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            // Modify the binding number, stageFlags, and descriptorCount according to your requirements
            VkDescriptorSetLayoutBinding uniformBufferBinding{};
            uniformBufferBinding.binding = binding;  // Choose the binding number for your uniform buffer object
            uniformBufferBinding.descriptorType = descriptorType;
            uniformBufferBinding.descriptorCount = count;
            uniformBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // used in both vertex and fragment shader
            bindings[binding] = uniformBufferBinding;
        }

        return *this;
    }

    // Build and return the Descriptor Set Layout.
    std::unique_ptr<lmDescriptorSetLayout> lmDescriptorSetLayout::Builder::build() const {
        return std::make_unique<lmDescriptorSetLayout>(device, bindings);
    }

    // *********************** Descriptor Set Layout ***********************

    // Constructor for lmDescriptorSetLayout with specified device and bindings.
    lmDescriptorSetLayout::lmDescriptorSetLayout(
        lmDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : device{ device }, bindings{ bindings }, descriptorSetLayout{ VK_NULL_HANDLE } {
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
            for (const auto& [key, value] : bindings) {
                setLayoutBindings.push_back(value);
            }

            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
            descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
            descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

            if (vkCreateDescriptorSetLayout(
                device.getDevice(),
                &descriptorSetLayoutInfo,
                nullptr,
                &descriptorSetLayout) != VK_SUCCESS) {
                    LOG_FATAL("Failed to create descriptor set layout!");
            }
    }

    // Destructor for lmDescriptorSetLayout.
    lmDescriptorSetLayout::~lmDescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(device.getDevice(), descriptorSetLayout, nullptr);
    }

    // *********************** Descriptor Pool Builder ***********************

    // Add a descriptor pool size with the given descriptor type and count to the Descriptor Pool Builder.
    lmDescriptorPool::Builder& lmDescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({ descriptorType, count });

        return *this;
    }

    // Set the flags for the descriptor pool.
    lmDescriptorPool::Builder& lmDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }

    // Set the maximum number of descriptor sets that can be allocated from the pool.
    lmDescriptorPool::Builder& lmDescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    // Build and return the Descriptor Pool.
    std::unique_ptr<lmDescriptorPool> lmDescriptorPool::Builder::build() const {
        return std::make_unique<lmDescriptorPool>(device, maxSets, poolFlags, poolSizes);
    }

    // *********************** Descriptor Pool ***********************

    // Constructor for lmDescriptorPool with specified device, maximum sets, pool flags, and pool sizes.
    lmDescriptorPool::lmDescriptorPool(
        lmDevice& device,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes)
        : device{ device } {
            VkDescriptorPoolCreateInfo descriptorPoolInfo{};
            descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            descriptorPoolInfo.pPoolSizes = poolSizes.data();
            descriptorPoolInfo.maxSets = maxSets;
            descriptorPoolInfo.flags = poolFlags;

            if (vkCreateDescriptorPool(
                device.getDevice(),
                &descriptorPoolInfo,
                nullptr,
                &descriptorPool) != VK_SUCCESS) {
                    LOG_FATAL("Failed to create descriptor pool!");
            }
    }

    // Destructor for lmDescriptorPool.
    lmDescriptorPool::~lmDescriptorPool() {
        vkDestroyDescriptorPool(device.getDevice(), descriptorPool, nullptr);
    }

    // Allocate a descriptor set from the descriptor pool and return it.
    bool lmDescriptorPool::allocateDescriptor(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(
            device.getDevice(),
            &allocInfo,
            &descriptor) != VK_SUCCESS) {
                return false;
        }

        return true;
    }

    // Free a vector of descriptor sets from the descriptor pool.
    void lmDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(
            device.getDevice(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    // Reset the descriptor pool.
    void lmDescriptorPool::resetPool() {
        vkResetDescriptorPool(device.getDevice(), descriptorPool, 0);
    }

    // *********************** Descriptor Writer ***********************

    // Constructor for lmDescriptorWriter with specified descriptor set layout and descriptor pool.
    lmDescriptorWriter::lmDescriptorWriter(lmDescriptorSetLayout& setLayout, lmDescriptorPool& pool)
        : setLayout{ setLayout }, pool{ pool } {}

    // Write a buffer descriptor to the descriptor set writer with the given binding and buffer information.
    lmDescriptorWriter& lmDescriptorWriter::writeBuffer(
        uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    // Write an image descriptor to the descriptor set writer with the given binding and image information.
    lmDescriptorWriter& lmDescriptorWriter::writeImage(
        uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    // Build a new descriptor set and allocate it from the descriptor pool.
    bool lmDescriptorWriter::build(VkDescriptorSet& set) {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);

        if (!success) {
            return false;
        }

        overwrite(set);
        return true;
    }

    // Overwrite an existing descriptor set with the descriptor writes in the writer.
    void lmDescriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }

        vkUpdateDescriptorSets(
            pool.device.getDevice(),
            static_cast<uint32_t>(writes.size()),
            writes.data(),
            0,
            nullptr);
    }

}  // namespace lm
