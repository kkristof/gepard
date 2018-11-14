/* Copyright (C) 2018, Gepard Graphics
 * Copyright (C) 2018, Kristof Kosztyo <kkristof@inf.u-szeged.hu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GEPARD_VULKAN_CONTAINERS_H
#define GEPARD_VULKAN_CONTAINERS_H

#include "gepard-vulkan-interface.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace gepard {

namespace vulkan {

class VkBaseElement
{
public:
    VkBaseElement();

    virtual void destroyElement(GepardVulkanInterface &vk, VkDevice &device, VkAllocationCallbacks* allocator);
};

class GepardVkImageElement : virtual public VkBaseElement
{
public:
    GepardVkImageElement(VkImage image, VkImageView imageView, VkDeviceMemory memory);

    virtual void destroyElement(GepardVulkanInterface &vk, VkDevice &device, VkAllocationCallbacks* allocator) override;
private:
    VkImage _image;
    VkImageView _imageView;
    VkDeviceMemory _imageMemory;
};

class GepardVkBufferElement : virtual public VkBaseElement
{
public:
    GepardVkBufferElement(VkBuffer buffer, VkDeviceMemory memory);

    virtual void destroyElement(GepardVulkanInterface &vk, VkDevice &device, VkAllocationCallbacks* allocator) override;
private:
    VkBuffer _buffer;
    VkDeviceMemory _memory;
};

class GepardVKPipelineElement : virtual public VkBaseElement
{
public:
    GepardVKPipelineElement(VkPipeline pipeline, VkPipelineLayout layout);

    virtual void destroyElement(GepardVulkanInterface &vk, VkDevice &device, VkAllocationCallbacks *allocator) override;
private:
    VkPipeline _pipeline;
    VkPipelineLayout _layout;
};

class GepardVulkanContainer
{
public:
    GepardVulkanContainer(GepardVulkanInterface &vk, VkDevice &device, VkAllocationCallbacks* allocator);
    ~GepardVulkanContainer();
    void addElement(VkBaseElement* element);
    void clear();
private:
    std::vector<VkBaseElement*> _elements;
    GepardVulkanInterface& _vk;
    VkDevice& _device;
    VkAllocationCallbacks* _allocator;
};


} // namespace vulkan

} // namespace gepard

#endif // GEPARD_VULKAN_CONTAINERS_H
