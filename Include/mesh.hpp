#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vertex.h"
#include "textureManager.hpp"
#include <memory>

struct MaterialData
{
  alignas(16) glm::vec3 ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
  alignas(16) glm::vec3 diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
  alignas(16) glm::vec3 specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
  alignas(16) glm::vec3 emissionColor = glm::vec3(0.0f, 0.0f, 0.0f);
  float shininess = 32.0f;
  float opacity = 1.0f;
  float refractiveIndex = 1.45f;
  int illuminationModel = 2;
  int hasTexture = 0;
};

class TextureManager;
class Renderer;
class Mesh
{
public:
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  int id;

  std::string texPath;

  MaterialData material;
  std::shared_ptr<TextureManager> textureManager;

  Mesh(Renderer &renderer, int *nextRenderingId, MaterialData material, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
  Mesh(std::shared_ptr<TextureManager> textureManager, int *nextRenderingId, MaterialData newMaterial, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);

  void initGraphics(Renderer &renderer, std::string texturePath);
  void initGraphics(Renderer &renderer); // only use if Mesh does not own the TextureManager
  void draw(Renderer *renderer, int currentFrame, glm::mat4 transformation, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer);
  void cleanup(VkDevice device, Renderer &renderer);

private:
  bool ownsTextureManager;
};