#include "debugDrawer.hpp"
#include "renderer.hpp"
#include <glm/glm.hpp>
#include "mesh.hpp"

void VulkanDebugDrawer::drawDebugLines(VkCommandBuffer commandBuffer, const std::vector<Vertex> &lines, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, int currentFrame)
{
  if (debugMode == 0)
    return;

  if (lines.empty())
    return;

  std::vector<Vertex> transformedLines;

  for (int i = 0; i < lines.size(); i += 2)
  {
    glm::vec3 fromVulkan = glm::vec3(-lines[i].pos.x, lines[i].pos.y, lines[i].pos.z);
    glm::vec3 toVulkan = glm::vec3(-lines[i + 1].pos.x, lines[i + 1].pos.y, lines[i + 1].pos.z);

    fromVulkan.x = -fromVulkan.x;
    toVulkan.x = -toVulkan.x;

    fromVulkan = glm::vec3(viewMatrix * glm::vec4(fromVulkan, 1.0f));
    toVulkan = glm::vec3(viewMatrix * glm::vec4(toVulkan, 1.0f));

    fromVulkan = glm::vec3(projectionMatrix * glm::vec4(fromVulkan, 1.0f));
    toVulkan = glm::vec3(projectionMatrix * glm::vec4(toVulkan, 1.0f));

    transformedLines.push_back(Vertex{fromVulkan, lines[i].color});
    transformedLines.push_back(Vertex{toVulkan, lines[i + 1].color});
  }

  // renderer.bufferManager.freeVertexBuffer(buffersIndex, renderer.deviceManager.device);
  renderer.bufferManager.createVertexBuffer(lines, buffersIndex, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);

  VkBuffer vertexBuffersArray[] = {renderer.bufferManager.vertexBuffers[buffersIndex]};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersArray, offsets);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipelineManager.pipelineLayout, 0, 1, &renderer.descriptorManager.descriptorSets[currentFrame + buffersIndex * renderer.MAX_FRAMES_IN_FLIGHT], 0, nullptr);

  renderer.bufferManager.updateUniformBuffer(currentFrame + buffersIndex * renderer.MAX_FRAMES_IN_FLIGHT, glm::mat4(1), viewMatrix, projectionMatrix);

  MaterialData materialData;
  materialData.diffuseColor = glm::vec3(0);
  materialData.hasTexture = false;

  vkCmdPushConstants(
      commandBuffer,
      renderer.pipelineManager.pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(MaterialData),
      &materialData);

  vkCmdDraw(commandBuffer, transformedLines.size(), 1, 0, 0);
}
VulkanDebugDrawer::VulkanDebugDrawer(Renderer &renderer, int &buffersIndex, bool debug) : debugMode(debug ? 1 : 0), renderer(renderer), buffersIndex(buffersIndex), textureManager(renderer.bufferManager, renderer)
{
  textureManager.createTextureImage("models/couch/diffuse.png", renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);
  textureManager.createTextureImageView(renderer.deviceManager.device);
  textureManager.createTextureSampler(renderer.deviceManager.device, renderer.deviceManager.physicalDevice);

  renderer.bufferManager.createUniformBuffers(renderer.MAX_FRAMES_IN_FLIGHT, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, 1);

  renderer.descriptorManager.addDescriptorSets(renderer.deviceManager.device, renderer.MAX_FRAMES_IN_FLIGHT, 1, textureManager.textureImageView, textureManager.textureSampler);
  buffersIndex++;
}