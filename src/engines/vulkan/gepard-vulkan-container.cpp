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

#include "gepard-vulkan-container.h"

namespace gepard {
namespace vulkan {

GepardVkImageElement::GepardVkImageElement(VkImage image, VkImageView imageView, VkDeviceMemory memory)
    : _image(image)
    , _imageView(imageView)
    , _imageMemory(memory)
{
}

void GepardVkImageElement::destroyElement(GepardVulkanInterface &vk, VkDevice &device, VkAllocationCallbacks *allocator)
{
    vk.vkDestroyImageView(device, _imageView, allocator);
    vk.vkFreeMemory(device, _imageMemory, allocator);
    vk.vkDestroyImage(device, _image, allocator);
}

VkBaseElement::VkBaseElement()
{
}

void VkBaseElement::destroyElement(GepardVulkanInterface &vk, VkDevice &device, VkAllocationCallbacks *allocator)
{
}

GepardVulkanContainer::GepardVulkanContainer(GepardVulkanInterface &vk, VkDevice &device, VkAllocationCallbacks *allocator)
    : _vk(vk)
    , _device(device)
    , _allocator(allocator)
{
}

GepardVulkanContainer::~GepardVulkanContainer()
{
    for (VkBaseElement* element : _elements) {
        element->destroyElement(_vk, _device, _allocator);
    }
}

void GepardVulkanContainer::addElement(VkBaseElement* element)
{
    _elements.push_back(element);
}

GepardVkBufferElement::GepardVkBufferElement(VkBuffer buffer, VkDeviceMemory memory)
    : _buffer(buffer)
    , _memory(memory)
{
}

void GepardVkBufferElement::destroyElement(GepardVulkanInterface &vk, VkDevice &device, VkAllocationCallbacks *allocator)
{
    vk.vkFreeMemory(device, _memory, allocator);
    vk.vkDestroyBuffer(device, _buffer, allocator);
}

GepardVKPipelineElement::GepardVKPipelineElement(VkPipeline pipeline, VkPipelineLayout layout)
    : _pipeline(pipeline)
    , _layout(layout)
{
}

void GepardVKPipelineElement::destroyElement(GepardVulkanInterface &vk, VkDevice &device, VkAllocationCallbacks *allocator)
{
    vk.vkDestroyPipeline(device, _pipeline, allocator);
    vk.vkDestroyPipelineLayout(device, _layout, allocator);
}

} // namespace vulkan
} // namespace gepard
