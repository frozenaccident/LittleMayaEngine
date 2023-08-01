#pragma once

#include "Device.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace lm {

    class lmDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(lmDevice& device) : device{ device } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<lmDescriptorSetLayout> build() const;

        private:
            lmDevice& device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        lmDescriptorSetLayout(
            lmDevice& lmDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~lmDescriptorSetLayout();
        lmDescriptorSetLayout(const lmDescriptorSetLayout&) = delete;
        lmDescriptorSetLayout& operator=(const lmDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        lmDevice& device;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class lmDescriptorWriter;
    };

    class lmDescriptorPool {
    public:
        class Builder {
        public:
            Builder(lmDevice& device) : device{ device } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<lmDescriptorPool> build() const;

        private:
            lmDevice& device;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        lmDescriptorPool(
            lmDevice& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~lmDescriptorPool();
        lmDescriptorPool(const lmDescriptorPool&) = delete;
        lmDescriptorPool& operator=(const lmDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
        void resetPool();

    private:
        lmDevice& device;
        VkDescriptorPool descriptorPool;

        friend class lmDescriptorWriter;
    };

    class lmDescriptorWriter {
    public:
        lmDescriptorWriter(lmDescriptorSetLayout& setLayout, lmDescriptorPool& pool);

        lmDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        lmDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        lmDescriptorSetLayout& setLayout;
        lmDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace lm
