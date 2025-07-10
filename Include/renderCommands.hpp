#ifndef RENDER_COMMANDS_H
#define RENDER_COMMANDS_H

#include <functional>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#ifndef RENDER_COMMAND
#define RENDER_COMMAND

struct RenderCommand
{
  std::function<void(VkCommandBuffer)> execute;
};

#endif
struct Vertex;
class VulkanDebugDrawer;
class GameObject;
class Text;
class Renderer;

RenderCommand makeGameObjectCommand(GameObject &gameObject, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj);
RenderCommand makeUICommand(GameObject &gameObject, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj);
RenderCommand makeTextCommand(Text &text, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj);
RenderCommand makeDebugCommand(VulkanDebugDrawer *drawer, Renderer *renderer, const std::vector<Vertex> &lines, glm::mat4 view, glm::mat4 proj, int currentFrame);

#endif