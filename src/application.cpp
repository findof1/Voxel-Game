#include "application.hpp"

std::vector<Vertex> getItemUIVertices(Item item)
{
  std::vector<Vertex> vertices = squareVertices;
  glm::vec2 start, size;
  size = glm::vec2(0.1);
  switch (item)
  {
  case Item::Dirt:
    start = glm::vec2(0);
    break;
  case Item::Grass_Dirt:
    start = glm::vec2(0.1, 0);
    break;
  case Item::Stone:
    start = glm::vec2(0.2, 0);
    break;
  case Item::Sand:
    start = glm::vec2(0.3, 0);
    break;
  case Item::Tree_Trunk:
    start = glm::vec2(0.4, 0);
    break;
  case Item::Tree_Leafes_Solid:
    start = glm::vec2(0.5, 0);
    break;
  default:
    start = glm::vec2(0);
    size = glm::vec2(1);
  }
  for (auto &vertex : vertices)
  {
    vertex.tileStart = start;
    vertex.tileSize = size;
  }
  return vertices;
}

void Application::run()
{
  std::function<void(Engine *, float)> updateFn =
      [this](Engine *engine, float dt)
  {
    this->update(engine, dt);
  };

  std::function<void(Engine *)> startFn =
      [this](Engine *engine)
  {
    this->start(engine);
  };

  engine.init("Game Engine", startFn, updateFn);

  MaterialData ground;
  ground.diffuseColor = {0.5, 0.5, 0.5};
  ground.hasTexture = 1;

  engine.createGameObject("skyBox", glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1000, 1000, 1000));
  engine.addMeshToObject("skyBox", ground, "textures/sky.png", cubeVerticesNoNormals, skyBoxIndices);

  MaterialData player;
  player.diffuseColor = {0.0, 0.0, 0.0};
  player.hasTexture = 1;

  engine.createGameObject("player", glm::vec3(0, 255, 0), glm::vec3(0, 0, 0), glm::vec3(0.8f, 1.5f, 0.8f));
  engine.addMeshToObject("player", player, "textures/wood.png", cubeVertices, cubeIndices);

  engine.createUIObject("crosshair", glm::vec3(engine.WIDTH / 2, -500, -7), glm::vec3(0), glm::vec3(20, 20, 1));
  engine.addMeshToObject("crosshair", player, "textures/crosshair.png", squareVertices, squareIndices);

  engine.createUIObject("hotbar", glm::vec3(engine.WIDTH / 2, -900, -7), glm::vec3(0), glm::vec3(480 * 1.5f, 48 * 1.5f, 1));
  engine.addMeshToObject("hotbar", player, "textures/hotbar.png", squareVertices, squareIndices);

  // engine.createUIObject("Dirt", glm::vec3(engine.WIDTH / 2 - 324, -900, -6), glm::vec3(0), glm::vec3(32 * 1.5f, 32 * 1.5f, 1));
  // engine.addMeshToObject("Dirt", player, "textures/itemAtlas.png", getItemUIVertices(Item::Dirt), squareIndices);

  for (int i = 0; i < inventory.items.size(); i++)
  {
    engine.createTextObject(std::to_string(i), " ", glm::vec3(engine.WIDTH / 2 - 320 + (i * 1.5f * 48), -924, -6), glm::vec3(0), glm::vec3(0.6f));
  }

  world.startWorker();

  world.startWorker();
  world.startWorker();
  engine.run();
  engine.shutdown();
}

void Application::update(Engine *engine, float dt)
{
  if (engine->input.keys[GLFW_KEY_ESCAPE])
    glfwSetWindowShouldClose(engine->window, GLFW_TRUE);

  glm::vec3 movementInput(0.0f);

  float moveSpeed = 3.0f;
  if (engine->input.keys[GLFW_KEY_LEFT_SHIFT])
    moveSpeed *= 1.5f;

  if (engine->input.keys[GLFW_KEY_W])
    movementInput += engine->camera.Front;
  if (engine->input.keys[GLFW_KEY_S])
    movementInput -= engine->camera.Front;

  glm::vec3 cameraRight = glm::normalize(glm::cross(engine->camera.Front, engine->camera.Up));
  if (engine->input.keys[GLFW_KEY_D])
    movementInput += cameraRight;
  if (engine->input.keys[GLFW_KEY_A])
    movementInput -= cameraRight;

  if (engine->input.keys[GLFW_KEY_SPACE])
  {
    if (!spaceClick)
    {
      playerVel.y = 5;
    }
    spaceClick = true;
  }
  else
  {
    spaceClick = false;
  }

  if (engine->input.keys[GLFW_KEY_0])
    inventory.selectedSlot = 9;
  if (engine->input.keys[GLFW_KEY_1])
    inventory.selectedSlot = 0;
  if (engine->input.keys[GLFW_KEY_2])
    inventory.selectedSlot = 1;
  if (engine->input.keys[GLFW_KEY_3])
    inventory.selectedSlot = 2;
  if (engine->input.keys[GLFW_KEY_4])
    inventory.selectedSlot = 3;
  if (engine->input.keys[GLFW_KEY_5])
    inventory.selectedSlot = 4;
  if (engine->input.keys[GLFW_KEY_6])
    inventory.selectedSlot = 5;
  if (engine->input.keys[GLFW_KEY_7])
    inventory.selectedSlot = 6;
  if (engine->input.keys[GLFW_KEY_8])
    inventory.selectedSlot = 7;
  if (engine->input.keys[GLFW_KEY_9])
    inventory.selectedSlot = 8;

  movementInput.y = 0.0f;

  if (glm::length(movementInput) > 0.0f)
    movementInput = glm::normalize(movementInput);

  playerVel.x = movementInput.x * moveSpeed;
  playerVel.z = movementInput.z * moveSpeed;

  playerVel.y -= 9 * dt;
  playerPos = playerPos + playerVel * dt;
  resolveCollisions(playerPos, playerVel, glm::vec3(0.4f, 0.9f, 0.4f));
  engine->gameObjects.at("player").position = playerPos;
  engine->gameObjects.at("skyBox").position = playerPos;

  updateCam(engine, dt);

  glm::vec3 rayOrigin = engine->camera.Position;
  glm::vec3 rayDirection = glm::normalize(engine->camera.getForward());

  glm::ivec3 blockHit, normalHit;
  BlockType blockType;
  if (raycastVoxel(rayOrigin, rayDirection, 8.0f, blockHit, normalHit, blockType))
  {
    std::vector<glm::vec3> corners = {
        {blockHit.x, blockHit.y, blockHit.z}, {blockHit.x + 1, blockHit.y, blockHit.z}, {blockHit.x + 1, blockHit.y + 1, blockHit.z}, {blockHit.x, blockHit.y + 1, blockHit.z}, {blockHit.x, blockHit.y, blockHit.z + 1}, {blockHit.x + 1, blockHit.y, blockHit.z + 1}, {blockHit.x + 1, blockHit.y + 1, blockHit.z + 1}, {blockHit.x, blockHit.y + 1, blockHit.z + 1}};

    auto addLine = [&](glm::vec3 a, glm::vec3 b)
    {
      engine->linesDrawer->debugLines.push_back(Vertex{a, glm::vec3(0.2f)});
      engine->linesDrawer->debugLines.push_back(Vertex{b, glm::vec3(0.2f)});
    };

    addLine(corners[0], corners[1]);
    addLine(corners[1], corners[2]);
    addLine(corners[2], corners[3]);
    addLine(corners[3], corners[0]);

    addLine(corners[4], corners[5]);
    addLine(corners[5], corners[6]);
    addLine(corners[6], corners[7]);
    addLine(corners[7], corners[4]);

    addLine(corners[0], corners[4]);
    addLine(corners[1], corners[5]);
    addLine(corners[2], corners[6]);
    addLine(corners[3], corners[7]);

    if (engine->input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT])
    {
      if (!leftMouseClick)
      {
        leftMouseClick = true;
        world.setBlock(blockHit.x, blockHit.y, blockHit.z, BlockType::Air);
        addItemToInventory(inventory, blockToItem(blockType), 1);
      }
    }
    else
    {
      leftMouseClick = false;
    }

    if (engine->input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT])
    {
      if (!rightMouseClick && inventory.items[inventory.selectedSlot].item != Item::None && inventory.items[inventory.selectedSlot].currentStackSize > 0)
      {
        rightMouseClick = true;
        glm::vec3 placeSpot = blockHit + normalHit;

        glm::vec3 blockMax = placeSpot + glm::vec3(1.0f);

        glm::vec3 min = playerPos - glm::vec3(0.4f, 0.9f, 0.4f);
        glm::vec3 max = playerPos + glm::vec3(0.4f, 0.9f, 0.4f);

        if (!AABBIntersect(min, max, placeSpot, blockMax))
        {
          inventory.items[inventory.selectedSlot].currentStackSize -= 1;
          if (inventory.items[inventory.selectedSlot].currentStackSize <= 0)
          {
            inventory.items[inventory.selectedSlot].makeTexture = true;
          }

          world.setBlock(placeSpot.x, placeSpot.y, placeSpot.z, itemToBlock(inventory.items[inventory.selectedSlot].item));
        }
      }
    }
    else
    {
      rightMouseClick = false;
    }
  }
  drawInventory();

  glm::ivec3 playerChunk = world.worldToChunkCoords(engine->camera.Position.x, 0, engine->camera.Position.z);

  int renderDistance = 4;

  for (int r = renderDistance; r >= 0; r--)
  {
    for (int dx = -r; dx <= r; dx++)
    {
      int dz = r;
      tryAddChunk(dx, dz, playerChunk);
      if (r != 0)
        tryAddChunk(dx, -dz, playerChunk);
    }

    for (int dz = -r + 1; dz <= r - 1; dz++)
    {
      int dx = r;
      tryAddChunk(dx, dz, playerChunk);
      if (r != 0)
        tryAddChunk(-dx, dz, playerChunk);
    }
  }

  for (auto it = world.chunks.begin(); it != world.chunks.end();)
  {
    glm::ivec3 chunkPos(it->first);
    glm::vec3 floatPos(chunkPos);
    glm::vec3 camPos(engine->camera.Position);
    float distSquared = std::pow(camPos.x - floatPos.x, 2) + std::pow(camPos.z - floatPos.z, 2);
    if (distSquared > std::pow((renderDistance * 1.5) * ChunkData::chunkSize, 2))
    {
      std::lock_guard<std::mutex> lock(world.chunkDeletionMutex);
      std::lock_guard<std::mutex> lock2(world.chunkMutex);
      world.unloadChunk(chunkPos);
      std::string identifier = std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z);
      engine->removeGameObject(identifier);
      it = world.chunks.erase(it);
    }
    else
    {
      it++;
    }
  }

  CompletedData result;
  const int maxUploadsPerFrame = 1;
  int uploads = 0;
  while (uploads < maxUploadsPerFrame && world.completedMeshes.pop(result))
  {
    auto &[chunkPos, vertices, indices] = result;

    if (!world.hasChunk(chunkPos))
    {
      continue;
    }

    std::string identifier = std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z);
    if (engine->gameObjects.find(identifier) != engine->gameObjects.end())
    {
      std::lock_guard<std::mutex> lock(world.chunkDeletionMutex);
      std::lock_guard<std::mutex> lock2(world.chunkMutex);
      engine->removeGameObject(identifier);
    }
    engine->createGameObject(identifier, chunkPos, glm::vec3(0), glm::vec3(1));

    MaterialData ground;
    ground.diffuseColor = {0.5, 0.5, 0.5};
    ground.hasTexture = 1;

    engine->addVoxelMeshToObject(identifier, ground, "textures/tiles.png", vertices, indices);

    uploads++;
  }
}

void Application::drawInventory()
{
  for (int i = 0; i < inventory.items.size(); i++)
  {
    if (inventory.items[i].item != Item::None)
    {
      if (inventory.items[i].currentStackSize <= 0 && inventory.items[i].makeTexture)
      {
        engine.removeUIObject("item" + std::to_string(i));
        inventory.items[i].makeTexture = false;
        inventory.items[i].item = Item::None;
      }

      if (inventory.items[i].makeTexture)
      {
        MaterialData item;
        item.diffuseColor = {0.0, 0.0, 0.0};
        item.hasTexture = 1;
        engine.removeUIObject("item" + std::to_string(i));
        engine.createUIObject("item" + std::to_string(i), glm::vec3(engine.WIDTH / 2 - 324 + (i * 48 * 1.5f), -900, -6), glm::vec3(0), glm::vec3(32 * 1.5f, 32 * 1.5f, 1));
        engine.addMeshToObject("item" + std::to_string(i), item, "textures/itemAtlas.png", getItemUIVertices(inventory.items[i].item), squareIndices);
        inventory.items[i].makeTexture = false;
      }

      if (inventory.items[i].currentStackSize > 1)
      {
        std::string stackSize = std::to_string(inventory.items[i].currentStackSize);
        engine.updateTextObject(std::to_string(i), stackSize);
      }
      else
      {
        engine.updateTextObject(std::to_string(i), " ");
      }
    }
  }
}

bool Application::raycastVoxel(glm::vec3 originInitial, glm::vec3 direction, float maxDistance, glm::ivec3 &hitBlock, glm::ivec3 &hitNormal, BlockType &blockType)
{
  constexpr float originEpsilon = 1e-2f;
  glm::vec3 origin = originInitial + glm::normalize(direction) * originEpsilon;
  glm::vec3 pos = glm::floor(origin);

  glm::vec3 deltaDist = glm::abs(glm::vec3(1.0f) / direction);
  glm::ivec3 step;
  glm::vec3 sideDist;
  for (int i = 0; i < 3; i++)
  {
    if (direction[i] > 0)
    {
      step[i] = 1;
      sideDist[i] = ((float(pos[i]) + 1.0f) - origin[i]) * deltaDist[i];
    }
    else
    {
      step[i] = -1;
      sideDist[i] = (origin[i] - float(pos[i])) * deltaDist[i];
    }
  }

  float distanceTraveled = 0.0f;
  glm::ivec3 normal(0);

  while (distanceTraveled < maxDistance)
  {
    int axis;
    if (sideDist.x < sideDist.y)
    {
      if (sideDist.x < sideDist.z)
        axis = 0; // x
      else
        axis = 2; // z
    }
    else
    {
      if (sideDist.y < sideDist.z)
        axis = 1; // y
      else
        axis = 2; // z
    }

    pos[axis] += step[axis];
    sideDist[axis] += deltaDist[axis];

    normal = glm::ivec3(0);
    normal[axis] = -step[axis];

    distanceTraveled = glm::length(glm::vec3(pos) - origin);
    BlockType block = world.getBlock(pos.x, pos.y, pos.z);
    if (block != BlockType::Air && block != BlockType::Nothing && block != BlockType::Water)
    {
      blockType = block;
      hitBlock = pos;
      hitNormal = normal;
      return true;
    }
  }

  return false;
}

void Application::tryAddChunk(int dx, int dz, glm::ivec3 playerChunk)
{
  glm::ivec3 chunkPos = playerChunk + glm::ivec3(dx * ChunkData::chunkSize, 0, dz * ChunkData::chunkSize);

  if (!world.chunkLoadQueue.has(chunkPos))
  {
    world.chunkLoadQueue.push(chunkPos);
  }
}

void Application::updateCam(Engine *engine, float dt)
{
  const float mouseSensitivity = 0.1f;
  static bool firstMouse = true;
  static double lastX = 0.0, lastY = 0.0;
  engine->camera.Position = playerPos + glm::vec3(0, 0.6, 0);

  engine->disableCursor();
  if (firstMouse)
  {
    lastX = engine->input.mouseX;
    lastY = engine->input.mouseY;
    firstMouse = false;
  }

  float xoffset = static_cast<float>(engine->input.mouseX - lastX);
  float yoffset = static_cast<float>(lastY - engine->input.mouseY);

  lastX = engine->input.mouseX;
  lastY = engine->input.mouseY;

  engine->rotateCamera(xoffset * mouseSensitivity, yoffset * mouseSensitivity);

  static float zoom = 45.0f;
  zoom -= engine->input.scrollOffsetY;
  if (zoom < 1.0f)
    zoom = 1.0f;
  if (zoom > 90.0f)
    zoom = 90.0f;
  engine->setCameraZoom(zoom);

  engine->input.scrollOffsetX = 0.0;
  engine->input.scrollOffsetY = 0.0;
}

void Application::resolveCollisions(glm::vec3 &position, glm::vec3 &velocity, const glm::vec3 &size)
{
  glm::vec3 min = position - size;
  glm::vec3 max = position + size;

  int x0 = floor(min.x);
  int x1 = floor(max.x);
  int y0 = floor(min.y);
  int y1 = floor(max.y);
  int z0 = floor(min.z);
  int z1 = floor(max.z);

  for (int x = x0; x <= x1; ++x)
    for (int y = y0; y <= y1; ++y)
      for (int z = z0; z <= z1; ++z)
      {
        BlockType block = world.getBlock(x, y, z);
        if (block != BlockType::Air && block != BlockType::Nothing && block != BlockType::Water)
        {
          glm::vec3 blockMin = glm::vec3(x, y, z);
          glm::vec3 blockMax = blockMin + glm::vec3(1.0f);

          if (AABBIntersect(min, max, blockMin, blockMax))
          {
            glm::vec3 penetration = getPenetration(min, max, blockMin, blockMax);

            if (fabs(penetration.x) < fabs(penetration.y) && fabs(penetration.x) < fabs(penetration.z))
            {
              position.x += -penetration.x;
              velocity.x = 0;
            }
            else if (fabs(penetration.y) < fabs(penetration.z))
            {
              position.y += -penetration.y;
              velocity.y = 0;
            }
            else
            {
              position.z += -penetration.z;
              velocity.z = 0;
            }

            min = position - size;
            max = position + size;
          }
        }
      }
}

glm::vec3 Application::getPenetration(glm::vec3 minA, glm::vec3 maxA, glm::vec3 minB, glm::vec3 maxB)
{
  glm::vec3 penetration(0.0f);
  if (maxA.x > minB.x && minA.x < maxB.x)
  {
    float left = maxB.x - minA.x;
    float right = maxA.x - minB.x;
    penetration.x = (left < right) ? -left : right;
  }
  if (maxA.y > minB.y && minA.y < maxB.y)
  {
    float down = maxB.y - minA.y;
    float up = maxA.y - minB.y;
    penetration.y = (down < up) ? -down : up;
  }
  if (maxA.z > minB.z && minA.z < maxB.z)
  {
    float front = maxB.z - minA.z;
    float back = maxA.z - minB.z;
    penetration.z = (front < back) ? -front : back;
  }
  return penetration;
}

bool Application::AABBIntersect(glm::vec3 minA, glm::vec3 maxA, glm::vec3 minB, glm::vec3 maxB)
{
  return (minA.x < maxB.x && maxA.x > minB.x &&
          minA.y < maxB.y && maxA.y > minB.y &&
          minA.z < maxB.z && maxA.z > minB.z);
}