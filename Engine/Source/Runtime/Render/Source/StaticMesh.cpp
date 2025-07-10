#include "StaticMesh.h"

#include "VulkanInitializers.hpp"
#include "VulkanTools.h"


StaticMesh::StaticMesh()
{
}

StaticMesh::~StaticMesh()
{
    uniformBufferVS.destroy();
    texture.destroy();
    delete body;
    delete model;
}

void StaticMesh::updateUniformBuffers()
{
    VK_CHECK_RESULT(uniformBufferVS.map())
    memcpy(uniformBufferVS.mapped, &uboVS, sizeof(uboVS));
    uniformBufferVS.unmap();
}

void StaticMesh::prepareUniformBuffers(VulkanDevice* vulkanDevice)
{
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &uniformBufferVS,
        sizeof(uboVS),
        &uboVS))
}

void StaticMesh::AllocateSet(VkDevice device, VkDescriptorPool descriptorPool,
                             VkDescriptorSetLayout descriptorSetLayout)
{
    // Descriptor set
    VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(
        descriptorPool, &descriptorSetLayout, 1);
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet))
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
                                              &uniformBufferVS.descriptor),
        vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                                              &texture.descriptor)
    };
    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0,
                           nullptr);
}

void StaticMesh::Tick(float DeltaTime)
{
}
