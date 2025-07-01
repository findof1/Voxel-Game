
#ifndef VERTEX_H
#define VERTEX_H
#include <vulkan/vulkan.h>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <iostream>
#include <vector>

struct Vertex
{
  glm::vec3 pos;
  glm::u8vec3 color;
  glm::u8vec2 texPos;
  glm::u8vec3 normal;
  glm::vec2 tileStart = glm::vec2(0, 0);
  glm::vec2 tileSize = glm::vec2(1, 1);
  glm::u8vec2 repeatCount = glm::u8vec2(1, 1);

  static VkVertexInputBindingDescription getBindingDescription()
  {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions()
  {
    std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R8G8B8_UNORM;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R8G8_UNORM;
    attributeDescriptions[2].offset = offsetof(Vertex, texPos);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R8G8B8_UNORM;
    attributeDescriptions[3].offset = offsetof(Vertex, normal);

    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(Vertex, tileStart);

    attributeDescriptions[5].binding = 0;
    attributeDescriptions[5].location = 5;
    attributeDescriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[5].offset = offsetof(Vertex, tileSize);

    attributeDescriptions[6].binding = 0;
    attributeDescriptions[6].location = 6;
    attributeDescriptions[6].format = VK_FORMAT_R8G8_UNORM;
    attributeDescriptions[6].offset = offsetof(Vertex, repeatCount);

    return attributeDescriptions;
  }
};

#endif