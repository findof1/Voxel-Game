#include "gameObject.hpp"
#include "renderer.hpp"

void GameObject::cleanup(Renderer &renderer)
{
  for (auto mesh : meshes)
  {
    mesh.cleanup(renderer.deviceManager.device, renderer);
  }
}