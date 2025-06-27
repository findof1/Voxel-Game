#include "mesh.hpp"
#include "bufferManager.hpp"
#include "descriptorManager.hpp"
#include "textureManager.hpp"
#include "renderer.hpp"
#include <vulkan/vulkan.h>
#include <tiny_obj_loader.h>
#include <glm/gtc/quaternion.hpp>

Mesh::Mesh(Renderer &renderer, int *nextRenderingId, MaterialData newMaterial, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) : vertices(std::move(vertices)), indices(std::move(indices)), material(newMaterial), textureManager(std::make_shared<TextureManager>(renderer.bufferManager, renderer))
{
  ownsTextureManager = true;
  id = *nextRenderingId;
  (*nextRenderingId)++;
}

Mesh::Mesh(std::shared_ptr<TextureManager> textureManager, int *nextRenderingId, MaterialData newMaterial, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) : vertices(std::move(vertices)), indices(std::move(indices)), material(newMaterial), textureManager(textureManager)
{
  ownsTextureManager = false;
  id = *nextRenderingId;
  (*nextRenderingId)++;
}

void Mesh::initGraphics(Renderer &renderer, std::string texturePath)
{

  texPath = texturePath;
  textureManager->createTextureImage(texturePath, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);

  textureManager->createTextureImageView(renderer.deviceManager.device);

  textureManager->createTextureSampler(renderer.deviceManager.device, renderer.deviceManager.physicalDevice);

  renderer.bufferManager.createVertexBuffer(vertices, id, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);

  renderer.bufferManager.createIndexBuffer(indices, id, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);

  renderer.bufferManager.createUniformBuffers(renderer.MAX_FRAMES_IN_FLIGHT, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, 1);

  renderer.descriptorManager.addDescriptorSets(renderer.deviceManager.device, renderer.MAX_FRAMES_IN_FLIGHT, 1, textureManager->textureImageView, textureManager->textureSampler);
}

void Mesh::initGraphics(Renderer &renderer)
{
  if (ownsTextureManager)
  {
    std::cerr << "Cannot call initGraphics(Renderer &renderer) if the Mesh owns the TextureManager" << std::endl;
    return;
  }

  renderer.bufferManager.createVertexBuffer(vertices, id, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);

  renderer.bufferManager.createIndexBuffer(indices, id, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);

  renderer.bufferManager.createUniformBuffers(renderer.MAX_FRAMES_IN_FLIGHT, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, 1);

  renderer.descriptorManager.addDescriptorSets(renderer.deviceManager.device, renderer.MAX_FRAMES_IN_FLIGHT, 1, textureManager->textureImageView, textureManager->textureSampler);
}

void Mesh::draw(Renderer *renderer, int currentFrame, glm::mat4 transformation, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer)
{

  VkBuffer vertexBuffersArray[] = {renderer->bufferManager.vertexBuffers[id]};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersArray, offsets);

  vkCmdBindIndexBuffer(commandBuffer, renderer->bufferManager.indexBuffers[id], 0, VK_INDEX_TYPE_UINT32);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.pipelineLayout, 0, 1, &renderer->descriptorManager.descriptorSets[currentFrame + id * renderer->MAX_FRAMES_IN_FLIGHT], 0, nullptr);

  renderer->bufferManager.updateUniformBuffer(currentFrame + id * renderer->MAX_FRAMES_IN_FLIGHT, transformation, view, projectionMatrix);

  vkCmdPushConstants(
      commandBuffer,
      renderer->pipelineManager.pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(MaterialData),
      &material);

  vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void Mesh::cleanup(VkDevice device, Renderer &renderer)
{
  if (ownsTextureManager)
  {
    textureManager->cleanup(device);
  }
  if (renderer.bufferManager.vertexBuffers.size() > id && renderer.bufferManager.vertexBuffers[id] != VK_NULL_HANDLE)
  {
    vkDestroyBuffer(device, renderer.bufferManager.vertexBuffers[id], nullptr);
    renderer.bufferManager.vertexBuffers[id] = VK_NULL_HANDLE;
  }

  if (renderer.bufferManager.indexBuffers.size() > id && renderer.bufferManager.indexBuffers[id] != VK_NULL_HANDLE)
  {
    vkDestroyBuffer(device, renderer.bufferManager.indexBuffers[id], nullptr);
    renderer.bufferManager.indexBuffers[id] = VK_NULL_HANDLE;
  }

  for (int i = 0; i < renderer.MAX_FRAMES_IN_FLIGHT; i++)
  {
    vkDestroyBuffer(device, renderer.bufferManager.uniformBuffers[id * renderer.MAX_FRAMES_IN_FLIGHT + i], nullptr);

    renderer.bufferManager.uniformBuffers[id * renderer.MAX_FRAMES_IN_FLIGHT + i] = VK_NULL_HANDLE;
  }

  uint32_t descriptorSetCount = renderer.MAX_FRAMES_IN_FLIGHT;
  uint32_t startIndex = id * renderer.MAX_FRAMES_IN_FLIGHT;
  if (renderer.descriptorManager.descriptorSets.size() >= startIndex + descriptorSetCount)
  {
    VkDescriptorSet *pDescriptorSets = renderer.descriptorManager.descriptorSets.data() + startIndex;
    VkResult result = vkFreeDescriptorSets(device, renderer.descriptorManager.descriptorPool, descriptorSetCount, pDescriptorSets);
    if (result != VK_SUCCESS)
    {
      std::cerr << "Failed to free descriptor sets for mesh id " << id << std::endl;
    }
  }
}