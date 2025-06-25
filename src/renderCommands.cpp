#include "renderCommands.hpp"
#include "debugDrawer.hpp"
#include "renderer.hpp"
#include <glm/glm.hpp>
#include "gameObject.hpp"

void setupViewportScissor(Renderer *renderer, VkCommandBuffer commandBuffer)
{
  VkViewport viewport{};

  viewport.x = 0.0f;
  viewport.y = 0.0f;

  viewport.width = static_cast<float>(renderer->swapchainManager.swapChainExtent.width);
  viewport.height = static_cast<float>(renderer->swapchainManager.swapChainExtent.height);

  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = renderer->swapchainManager.swapChainExtent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void setupViewportScissor(VkCommandBuffer commandBuffer, VkExtent2D extent)
{
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(extent.width);
  viewport.height = static_cast<float>(extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = extent;

  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void setTriangleTopology(Renderer *renderer, VkCommandBuffer commandBuffer)
{
  if (!renderer->fpCmdSetPrimitiveTopology)
  {
    PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT =
        (PFN_vkCmdSetPrimitiveTopologyEXT)vkGetDeviceProcAddr(renderer->deviceManager.device, "vkCmdSetPrimitiveTopologyEXT");

    if (!vkCmdSetPrimitiveTopologyEXT)
    {
      std::cerr << "vkCmdSetPrimitiveTopology is not available and failed to load vkCmdSetPrimitiveTopologyEXT!" << std::endl;
    }
    else
    {
      vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    }
  }
  else
  {
    vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  }
}

void setLineListTopology(Renderer *renderer, VkCommandBuffer commandBuffer)
{
  if (!renderer->fpCmdSetPrimitiveTopology)
  {
    PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT =
        (PFN_vkCmdSetPrimitiveTopologyEXT)vkGetDeviceProcAddr(renderer->deviceManager.device, "vkCmdSetPrimitiveTopologyEXT");

    if (!vkCmdSetPrimitiveTopologyEXT)
    {
      std::cerr << "vkCmdSetPrimitiveTopology is not available and failed to load vkCmdSetPrimitiveTopologyEXT!" << std::endl;
    }
    else
    {
      vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    }
  }
  else
  {
    vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
  }
}

void setPointListTopology(Renderer *renderer, VkCommandBuffer commandBuffer)
{
  if (!renderer->fpCmdSetPrimitiveTopology)
  {
    PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT =
        (PFN_vkCmdSetPrimitiveTopologyEXT)vkGetDeviceProcAddr(renderer->deviceManager.device, "vkCmdSetPrimitiveTopologyEXT");

    if (!vkCmdSetPrimitiveTopologyEXT)
    {
      std::cerr << "vkCmdSetPrimitiveTopology is not available and failed to load vkCmdSetPrimitiveTopologyEXT!" << std::endl;
    }
    else
    {
      vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    }
  }
  else
  {
    vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
  }
}

void enableDepthWrite(Renderer *renderer, VkCommandBuffer commandBuffer)
{
  if (!renderer->vkCmdSetDepthWriteEnableEXT)
  {
    std::cerr << "vkCmdSetPrimitiveTopology is not available and failed to load vkCmdSetPrimitiveTopologyEXT!" << std::endl;
  }
  else
  {
    renderer->vkCmdSetDepthWriteEnableEXT(commandBuffer, VK_TRUE);
  }
}

void disableDepthWrite(Renderer *renderer, VkCommandBuffer commandBuffer)
{
  if (!renderer->vkCmdSetDepthWriteEnableEXT)
  {
    std::cerr << "vkCmdSetPrimitiveTopology is not available and failed to load vkCmdSetPrimitiveTopologyEXT!" << std::endl;
  }
  else
  {
    renderer->vkCmdSetDepthWriteEnableEXT(commandBuffer, VK_FALSE);
  }
}

RenderCommand makeGameObjectCommand(GameObject &gameObject, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj)
{
  return {
      [renderer, &gameObject, currentFrame, view, proj](VkCommandBuffer cmdBuf)
      {
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.graphicsPipeline);
        setTriangleTopology(renderer, cmdBuf);
        enableDepthWrite(renderer, cmdBuf);

        setupViewportScissor(renderer, cmdBuf);
        VkViewport viewport{};

        glm::mat4 transformation = glm::mat4(1.0f);
        transformation = glm::translate(transformation, gameObject.position);
        transformation = glm::rotate(transformation, glm::radians(gameObject.rotationZYX.x), glm::vec3(0.0f, 0.0f, 1.0f));
        transformation = glm::rotate(transformation, glm::radians(gameObject.rotationZYX.y), glm::vec3(0.0f, 1.0f, 0.0f));
        transformation = glm::rotate(transformation, glm::radians(gameObject.rotationZYX.z), glm::vec3(1.0f, 0.0f, 0.0f));
        transformation = glm::scale(transformation, gameObject.scale);

        for (auto &mesh : gameObject.meshes)
        {
          mesh.draw(renderer, currentFrame, transformation, view, proj, cmdBuf);
        }
      }};
}

RenderCommand makeDebugCommand(VulkanDebugDrawer *drawer, Renderer *renderer, const std::vector<Vertex> &lines, glm::mat4 view, glm::mat4 proj, int currentFrame)
{
  return {
      [=](VkCommandBuffer cmdBuf)
      {
        if (drawer)
        {
          vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.graphicsPipeline);
          setLineListTopology(renderer, cmdBuf);
          enableDepthWrite(renderer, cmdBuf);

          setupViewportScissor(renderer, cmdBuf);
          drawer->drawDebugLines(cmdBuf, lines, view, proj, currentFrame);
        }
      }};
}
