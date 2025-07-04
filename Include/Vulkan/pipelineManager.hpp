#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
class SwapchainManager;
class DescriptorManager;

class PipelineManager
{
public:
  VkRenderPass renderPass;

  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  SwapchainManager &swapchainManager;
  DescriptorManager &descriptorManager;
  std::vector<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT};
  PipelineManager(SwapchainManager &swapchainManager, DescriptorManager &descriptorManager) : swapchainManager(swapchainManager), descriptorManager(descriptorManager)
  {
  }
  ~PipelineManager()
  {
  }
  void createRenderPass(VkDevice device, VkPhysicalDevice physicalDevice);
  void createGraphicsPipeline(VkDevice device);
  void cleanup(VkDevice device);

private:
  static std::vector<char> readFile(const std::string &filename);
  static VkShaderModule createShaderModule(const std::vector<char> &code, VkDevice device);
};