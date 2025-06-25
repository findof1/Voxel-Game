#include "deviceManager.hpp"
#include "utils.h"
#include <set>
#include <stdexcept>
#include "swapchainManager.hpp"
#include <ostream>
#include <iostream>

void DeviceManager::createLogicalDevice(bool enableValidationLayers, const std::vector<const char *> &deviceExtensions, const std::vector<const char *> &validationLayers, VkQueue *presentQueue, VkQueue *graphicsQueue, VkQueue *computeQueue)
{
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice, swapchainManager.surface);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value(), indices.computeFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies)
  {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkDeviceQueueCreateInfo queueCreateInfo{};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
  queueCreateInfo.queueCount = 1;

  queueCreateInfo.pQueuePriorities = &queuePriority;

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{};
  extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
  extendedDynamicStateFeatures.extendedDynamicState = VK_TRUE;

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceFeatures.robustBufferAccess = VK_TRUE;

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pNext = &extendedDynamicStateFeatures;

  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (enableValidationLayers)
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(device, indices.presentFamily.value(), 0, presentQueue);
  vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, graphicsQueue);
  vkGetDeviceQueue(device, indices.computeFamily.value(), 0, computeQueue);
}

void DeviceManager::pickPhysicalDevice(VkInstance instance, const std::vector<const char *> &deviceExtensions)
{

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

  if (deviceCount == 0)
  {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (const auto device : devices)
  {
    if (isDeviceSuitable(device, deviceExtensions))
    {
      physicalDevice = device;
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE)
  {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

bool DeviceManager::isDeviceSuitable(VkPhysicalDevice device, const std::vector<const char *> &deviceExtensions)
{
  QueueFamilyIndices indices = findQueueFamilies(device, swapchainManager.surface);

  bool extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions);

  bool swapChainAdequate = false;
  if (extensionsSupported)
  {
    SwapChainSupportDetails swapChainSupport = swapchainManager.querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;

  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{};
  extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;

  VkPhysicalDeviceFeatures2 deviceFeatures2{};
  deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  deviceFeatures2.pNext = &extendedDynamicStateFeatures;

  vkGetPhysicalDeviceFeatures2(device, &deviceFeatures2);

  if (!extendedDynamicStateFeatures.extendedDynamicState)
  {
    std::cerr << "Device does not support extended dynamic state!" << std::endl;
    return false;
  }

  return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy && supportedFeatures.robustBufferAccess;
}

bool DeviceManager::checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char *> &deviceExtensions)
{
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto &extension : availableExtensions)
  {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}