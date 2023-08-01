#include "Buffer.h"

#include <cassert>
#include <cstring>

namespace lm {

    /**
     * @brief Returns the minimum instance size required to be compatible with the device's minOffsetAlignment.
     *
     * @param instanceSize The size of an instance
     * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (e.g. minUniformBufferOffsetAlignment)
     *
     * @return The aligned instance size based on the device's minOffsetAlignment
     */
    VkDeviceSize lmBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    /**
     * Constructor for lmBuffer class.
     *
     * @param device The Vulkan device
     * @param instanceSize The size of a single instance in the buffer
     * @param instanceCount The number of instances to be stored in the buffer
     * @param usageFlags Flags that define how the buffer will be used (e.g. VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
     * @param memoryPropertyFlags Flags that define the memory properties of the buffer (e.g. VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
     * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (e.g. minUniformBufferOffsetAlignment)
     */
    lmBuffer::lmBuffer(
        lmDevice& device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment)
        : deviceInstance{ device },
        instanceSize{ instanceSize },
        instanceCount{ instanceCount },
        usageFlags{ usageFlags },
        memoryPropertyFlags{ memoryPropertyFlags } {
        alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        bufferSize = alignmentSize * instanceCount;
        device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
    }

    /**
     * Destructor for lmBuffer class.
     */
    lmBuffer::~lmBuffer() {
        unmap();
        vkDestroyBuffer(deviceInstance.getDevice(), buffer, nullptr);
        vkFreeMemory(deviceInstance.getDevice(), memory, nullptr);
    }

    /**
     * Map a memory range of this buffer. If successful, 'mapped' points to the specified buffer range.
     *
     * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
     * @param offset (Optional) Byte offset from the beginning
     *
     * @return VkResult of the buffer mapping call
     */
    VkResult lmBuffer::map(VkDeviceSize size, VkDeviceSize offset) {
        assert(buffer && memory && "Called map on buffer before create");
        return vkMapMemory(deviceInstance.getDevice(), memory, offset, size, 0, &mapped);
    }

    /**
     * Unmap a previously mapped memory range.
     */
    void lmBuffer::unmap() {
        if (mapped) {
            vkUnmapMemory(deviceInstance.getDevice(), memory);
            mapped = nullptr;
        }
    }

    /**
     * Copies the specified data to the mapped buffer.
     *
     * @param data Pointer to the data to copy
     * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer range.
     * @param offset (Optional) Byte offset from the beginning of the mapped region
     */
    void lmBuffer::writeToBuffer(const void* data, VkDeviceSize size, VkDeviceSize offset) {
        assert(mapped && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE) {
            memcpy(mapped, data, bufferSize);
        }
        else {
            char* memOffset = (char*)mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    /**
     * Flush a memory range of the buffer to make it visible to the device.
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
     * @param offset (Optional) Byte offset from the beginning
     *
     * @return VkResult of the flush call
     */
    VkResult lmBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(deviceInstance.getDevice(), 1, &mappedRange);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host.
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
     * @param offset (Optional) Byte offset from the beginning
     *
     * @return VkResult of the invalidate call
     */
    VkResult lmBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(deviceInstance.getDevice(), 1, &mappedRange);
    }

    /**
     * Create a buffer info descriptor for the specified range.
     *
     * @param size (Optional) Size of the memory range of the descriptor
     * @param offset (Optional) Byte offset from the beginning
     *
     * @return VkDescriptorBufferInfo of the specified offset and range
     */
    VkDescriptorBufferInfo lmBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        return VkDescriptorBufferInfo{
            buffer,
            offset,
            size,
        };
    }

    /**
     * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize.
     *
     * @param data Pointer to the data to copy
     * @param index Used in offset calculation
     */
    void lmBuffer::writeToIndex(void* data, int index) {
        writeToBuffer(data, instanceSize, index * alignmentSize);
    }

    /**
     * Flush the memory range at index * alignmentSize of the buffer to make it visible to the device.
     *
     * @param index Used in offset calculation
     *
     * @return VkResult of the flush call
     */
    VkResult lmBuffer::flushIndex(int index) {
        return flush(alignmentSize, index * alignmentSize);
    }

    /**
     * Create a buffer info descriptor for the specified index.
     *
     * @param index Specifies the region given by index * alignmentSize
     *
     * @return VkDescriptorBufferInfo for the instance at the index
     */
    VkDescriptorBufferInfo lmBuffer::descriptorInfoForIndex(int index) {
        return descriptorInfo(alignmentSize, index * alignmentSize);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host.
     *
     * @note Only required for non-coherent memory
     *
     * @param index Specifies the region to invalidate: index * alignmentSize
     *
     * @return VkResult of the invalidate call
     */
    VkResult lmBuffer::invalidateIndex(int index) {
        return invalidate(alignmentSize, index * alignmentSize);
    }

}  // namespace lm
