#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
class BufferManager;
class Renderer;
struct FT_Bitmap_;
typedef FT_Bitmap_ FT_Bitmap;
class TextureManager
{
public:
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;
  BufferManager &bufferManager;
  Renderer &renderer;
  TextureManager(BufferManager &bufferManager, Renderer &renderer) : bufferManager(bufferManager), renderer(renderer)
  {
  }
  ~TextureManager()
  {
  }
  void createTextureImageView(VkDevice device);
  void createTextTextureImageView(VkDevice device);

  void createTextureImage(std::string texturePath, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
  void createTextureImage(const FT_Bitmap &bitmap, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
  void createTextureImage(const std::vector<uint8_t> &textureData, int texWidth, int texHeight, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

  void createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice);

  void updateTexture(std::string newTexturePath, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
  void cleanup(VkDevice device);
};