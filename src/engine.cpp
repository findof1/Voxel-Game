#include "engine.hpp"
#include "renderer.hpp"
#include "renderCommands.hpp"
#include "debugDrawer.hpp"
#include <tiny_obj_loader.h>
#include <algorithm>

void Engine::initWindow(std::string windowName)
{
  glfwInit();

  // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window = glfwCreateWindow(WIDTH, HEIGHT, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetScrollCallback(window, scroll_callback);
  renderer.window = window;
  renderer.swapchainManager.setWindow(window);
}

void Engine::disableCursor()
{
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Engine::enableCursor()
{
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Engine::init(std::string windowName, std::function<void(Engine *)> startFn, std::function<void(Engine *, float)> updateFn)
{
  start = std::move(startFn);
  update = std::move(updateFn);

  initWindow(windowName);
  renderer.initVulkan();
  linesDrawer = new VulkanDebugDrawer(renderer, nextRenderingId, true);

  voxelTextureAtlas = std::make_shared<TextureManager>(renderer.bufferManager, renderer);
  voxelTextureAtlas->createTextureImage("textures/newAtlas.png", renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);

  voxelTextureAtlas->createTextureImageView(renderer.deviceManager.device);

  voxelTextureAtlas->createTextureSampler(renderer.deviceManager.device, renderer.deviceManager.physicalDevice);
}

void Engine::run()
{
  float lastFrame = 0.0f;
  float deltaTime = 0.0f;

  start(this);

  while (isRunning && !glfwWindowShouldClose(window))
  {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (autoFreeCam)
      updateFreeCam(deltaTime);

    update(this, deltaTime);

    render();

    glfwPollEvents();
  }
  vkDeviceWaitIdle(renderer.deviceManager.device);
}

void Engine::clearHierarchy()
{
  vkQueueWaitIdle(renderer.graphicsQueue);

  gameObjects.clear();
}

void Engine::shutdown()
{
  clearHierarchy();
  renderer.renderQueue.clear();
  renderer.cleanup();
}

void Engine::render()
{
  renderer.renderQueue.clear();
  glm::mat4 view = camera.getViewMatrix();
  glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / HEIGHT, 0.1f, 10000.0f);
  glm::mat4 ortho = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT, 0.05f, 10.0f);

  glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::mat4 staticView = glm::lookAt(cameraPos, cameraTarget, cameraUp);

  for (auto &[key, gameObject] : gameObjects)
  {
    renderer.renderQueue.push_back(makeGameObjectCommand(gameObject, &renderer, renderer.getCurrentFrame(), view, proj));
  }

  std::vector<std::reference_wrapper<GameObject>> uiList;
  for (auto &[key, obj] : uiObjects)
  {
    uiList.push_back(obj);
  }

  std::sort(uiList.begin(), uiList.end(),
            [&](const GameObject &a, const GameObject &b)
            {
              return abs(b.position.z) < abs(a.position.z);
            });

  for (auto &uiObject : uiList)
  {
    renderer.renderQueue.push_back(makeUICommand(uiObject, &renderer, renderer.getCurrentFrame(), staticView, ortho));
  }

  for (auto &[key, text] : textObjects)
  {
    renderer.renderQueue.push_back(makeTextCommand(text, &renderer, renderer.getCurrentFrame(), staticView, ortho));
  }

  renderer.renderQueue.push_back(makeDebugCommand(linesDrawer, &renderer, linesDrawer->debugLines, view, proj, renderer.getCurrentFrame()));
  renderer.drawFrame();
  linesDrawer->clearLines();
}

void Engine::updateFreeCam(float dt)
{
  float moveSpeed = 15.0f;
  const float mouseSensitivity = 0.1f;
  static bool firstMouse = true;
  static double lastX = 0.0, lastY = 0.0;
  if (input.keys[GLFW_KEY_LEFT_SHIFT])
    moveSpeed *= 2;

  if (input.keys[GLFW_KEY_W])
    moveCameraForwards(+moveSpeed * dt);
  if (input.keys[GLFW_KEY_S])
    moveCameraForwards(-moveSpeed * dt);
  if (input.keys[GLFW_KEY_D])
    moveCameraRight(+moveSpeed * dt);
  if (input.keys[GLFW_KEY_A])
    moveCameraRight(-moveSpeed * dt);
  if (input.keys[GLFW_KEY_SPACE])
    moveCamera({0.0f, +moveSpeed * dt, 0.0f});
  if (input.keys[GLFW_KEY_LEFT_CONTROL])
    moveCamera({0.0f, -moveSpeed * dt, 0.0f});

  if (input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT])
  {
    disableCursor();
    if (firstMouse)
    {
      lastX = input.mouseX;
      lastY = input.mouseY;
      firstMouse = false;
    }

    float xoffset = static_cast<float>(input.mouseX - lastX);
    float yoffset = static_cast<float>(lastY - input.mouseY);

    lastX = input.mouseX;
    lastY = input.mouseY;

    rotateCamera(xoffset * mouseSensitivity, yoffset * mouseSensitivity);
  }
  else
  {
    enableCursor();
    firstMouse = true;
  }

  static float zoom = 45.0f;
  zoom -= input.scrollOffsetY;
  if (zoom < 1.0f)
    zoom = 1.0f;
  if (zoom > 90.0f)
    zoom = 90.0f;
  setCameraZoom(zoom);

  input.scrollOffsetX = 0.0;
  input.scrollOffsetY = 0.0;
}

void Engine::createGameObject(std::string identifier, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
  if (gameObjects.find(identifier) != gameObjects.end())
  {
    return;
  }

  GameObject gameObject;
  gameObject.position = position;
  gameObject.rotationZYX = rotation;
  gameObject.scale = scale;
  gameObjects.emplace(identifier, std::move(gameObject));
}

void Engine::createUIObject(std::string identifier, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
  if (uiObjects.find(identifier) != uiObjects.end())
  {
    return;
  }

  GameObject gameObject;
  gameObject.position = position;
  gameObject.rotationZYX = rotation;
  gameObject.scale = scale;
  uiObjects.emplace(identifier, std::move(gameObject));
}

void Engine::createTextObject(std::string identifier, std::string text, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
  if (textObjects.find(identifier) != textObjects.end())
  {
    return;
  }

  textObjects.emplace(identifier, Text(renderer, &nextRenderingId, text, position, rotation, scale));
}
void Engine::hideTextObject(std::string identifier, bool hide)
{
  if (textObjects.find(identifier) == textObjects.end())
  {
    return;
  }

  textObjects.at(identifier).hide = hide;
}
void Engine::updateTextObject(std::string identifier, std::string text)
{
  if (textObjects.find(identifier) == textObjects.end())
  {
    return;
  }
  textObjects.at(identifier).updateText(text, renderer);
}
void Engine::updateTextObject(std::string identifier, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
  if (textObjects.find(identifier) == textObjects.end())
  {
    return;
  }
  textObjects.at(identifier).position = position;
  textObjects.at(identifier).rotation = rotation;
  textObjects.at(identifier).scale = scale;
}

void Engine::removeGameObject(std::string identifier)
{
  if (gameObjects.find(identifier) == gameObjects.end())
  {
    return;
  }

  gameObjects.at(identifier).cleanup(renderer);
  gameObjects.erase(identifier);
}
void Engine::removeUIObject(std::string identifier)
{
  if (uiObjects.find(identifier) == uiObjects.end())
  {
    return;
  }

  uiObjects.at(identifier).cleanup(renderer);
  uiObjects.erase(identifier);
}

void Engine::addMeshToObject(std::string identifier, MaterialData material, const std::string &texturePath, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
{
  auto it = gameObjects.find(identifier);
  if (it == gameObjects.end())
  {
    it = uiObjects.find(identifier);
    if (it == uiObjects.end())
    {
      return;
    }
  }
  GameObject &gameObject = it->second;

  Mesh mesh(renderer, &nextRenderingId, material, vertices, indices);
  mesh.initGraphics(renderer, texturePath.empty() ? "models/couch/diffuse.png" : texturePath);
  gameObject.meshes.emplace_back(std::move(mesh));
}

void Engine::addVoxelMeshToObject(std::string identifier, MaterialData material, const std::string &texturePath, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
{
  auto it = gameObjects.find(identifier);
  if (it == gameObjects.end())
  {
    return;
  }
  GameObject &gameObject = it->second;

  Mesh mesh(voxelTextureAtlas, &nextRenderingId, material, vertices, indices);
  mesh.initGraphics(renderer);
  gameObject.meshes.emplace_back(std::move(mesh));
}

void Engine::loadModel(std::string identifier, const std::string objPath, const std::string mtlPath)
{
  auto it = gameObjects.find(identifier);
  if (it == gameObjects.end())
  {
    return;
  }
  GameObject &gameObject = it->second;

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;

  bool ret = tinyobj::LoadObj(
      &attrib,
      &shapes,
      &materials,
      &err,
      objPath.c_str(), mtlPath.c_str());

  if (!ret)
  {
    throw std::runtime_error("Failed to load OBJ: " + err);
  }

  for (const auto &shape : shapes)
  {
    std::vector<Vertex> meshVertices;
    std::vector<uint32_t> meshIndices;
    for (const auto &index : shape.mesh.indices)
    {
      Vertex vertex{};

      size_t vIdx = 3 * index.vertex_index;
      if (vIdx + 2 >= attrib.vertices.size())
      {
        throw std::runtime_error("Vertex index out of range in model: " + objPath);
      }
      vertex.pos = {
          attrib.vertices[vIdx + 0],
          attrib.vertices[vIdx + 1],
          attrib.vertices[vIdx + 2]};

      if (!attrib.normals.empty() && (3 * index.normal_index + 2 < attrib.normals.size()))
      {
        vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2]};
      }
      else
      {
        vertex.normal = {0.0f, 0.0f, 0.0f};
      }

      if (!attrib.texcoords.empty() && (2 * index.texcoord_index + 1 < attrib.texcoords.size()))
      {
        vertex.texPos = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
      }
      else
      {
        vertex.texPos = {0.0f, 0.0f};
      }

      vertex.color = {-1.0f, -1.0f, -1.0f};

      meshVertices.push_back(vertex);
      meshIndices.push_back(static_cast<uint32_t>(meshIndices.size()));
    }
    MaterialData material;
    material.diffuseColor = {0.5f, 0.5f, 0.5f};
    material.hasTexture = 1;
    std::string texturePath = "";
    std::string fullPath = "";

    if (!materials.empty() && !shape.mesh.material_ids.empty())
    {
      int matId = shape.mesh.material_ids[0];
      if (matId < 0 || matId >= static_cast<int>(materials.size()))
      {
        throw std::runtime_error("Material index out of range in model: " + objPath);
      }

      if (!materials[matId].diffuse_texname.empty())
      {
        texturePath = materials[matId].diffuse_texname;
        fullPath = mtlPath + texturePath;
      }

      material.diffuseColor = {
          materials[matId].diffuse[0],
          materials[matId].diffuse[1],
          materials[matId].diffuse[2]};

      material.specularColor = {
          materials[matId].specular[0],
          materials[matId].specular[1],
          materials[matId].specular[2]};

      material.ambientColor = {
          materials[matId].ambient[0],
          materials[matId].ambient[1],
          materials[matId].ambient[2]};

      material.shininess = materials[matId].shininess;
      material.emissionColor = {
          materials[matId].emission[0],
          materials[matId].emission[1],
          materials[matId].emission[2]};

      material.refractiveIndex = materials[matId].ior;
      material.illuminationModel = materials[matId].illum;

      material.opacity = (materials[matId].diffuse[3] != 0) ? materials[matId].diffuse[3] : 1.0f;
    }

    if (texturePath.empty())
    {
      material.hasTexture = 0;
      fullPath = "models/couch/diffuse.png";
    }

    Mesh mesh(renderer, &nextRenderingId, material, meshVertices, meshIndices);

    try
    {
      mesh.initGraphics(renderer, fullPath);
    }
    catch (const std::exception &e)
    {
      throw std::runtime_error("Mesh graphics initialization failed: " + std::string(e.what()));
    }
    gameObject.meshes.push_back(std::move(mesh));
  }
}
