#pragma once

#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include "vertex.h"
#include "textureManager.hpp"

class Renderer;
class VulkanDebugDrawer
{
public:
  std::vector<Vertex> debugLines;
  int debugMode;
  Renderer &renderer;
  const int buffersIndex;
  TextureManager textureManager;

  VulkanDebugDrawer(Renderer &renderer, int &buffersIndex, bool debug);

  void drawDebugLines(VkCommandBuffer commandBuffer, const std::vector<Vertex> &lines, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, int currentFrame);

  void drawLine(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &color)
  {
    debugLines.push_back({from, color});
    debugLines.push_back({to, color});
  }

  void drawContactPoint(const glm::vec3 &pointOnB, const glm::vec3 &normalOnB, float distance, int lifeTime, const glm::vec3 &color)
  {
    drawLine(pointOnB, pointOnB + glm::vec3(normalOnB.x * distance * 5, normalOnB.y * distance * 5, normalOnB.z * distance * 5), color);
  }

  void reportErrorWarning(const char *warningString)
  {
    std::cerr << "Debug Warning: " << warningString << std::endl;
  }

  void setDebugMode(int debugMode)
  {
    this->debugMode = debugMode;
  }

  int getDebugMode() const
  {
    return debugMode;
  }

  void clearLines()
  {
    debugLines.clear();
  }
};