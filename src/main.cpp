#include <vulkan/vulkan.h>
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "engine.hpp"
#include "primitives.hpp"
#include <world.hpp>

#include <chrono>

glm::vec3 playerPos(0, 255, 0);
glm::vec3 playerVel(0);
World world;

bool AABBIntersect(glm::vec3 minA, glm::vec3 maxA, glm::vec3 minB, glm::vec3 maxB)
{
    return (minA.x < maxB.x && maxA.x > minB.x &&
            minA.y < maxB.y && maxA.y > minB.y &&
            minA.z < maxB.z && maxA.z > minB.z);
}

glm::vec3 getPenetration(glm::vec3 minA, glm::vec3 maxA, glm::vec3 minB, glm::vec3 maxB)
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

void resolveCollisions(glm::vec3 &position, glm::vec3 &velocity, const glm::vec3 &size)
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

void start(Engine *engine)
{
}

void updateCam(Engine *engine, float dt)
{
    const float mouseSensitivity = 0.1f;
    static bool firstMouse = true;
    static double lastX = 0.0, lastY = 0.0;
    engine->camera.Position = playerPos + glm::vec3(0, 0.6, 0);
    /*
    if (engine->input.keys[GLFW_KEY_LEFT_SHIFT])
        moveSpeed *= 2;

    if (engine->input.keys[GLFW_KEY_W])
        engine->moveCameraForwards(+moveSpeed * dt);
    if (engine->input.keys[GLFW_KEY_S])
        engine->moveCameraForwards(-moveSpeed * dt);
    if (engine->input.keys[GLFW_KEY_D])
        engine->moveCameraRight(+moveSpeed * dt);
    if (engine->input.keys[GLFW_KEY_A])
        engine->moveCameraRight(-moveSpeed * dt);
    if (engine->input.keys[GLFW_KEY_SPACE])
        engine->moveCamera({0.0f, +moveSpeed * dt, 0.0f});
    if (engine->input.keys[GLFW_KEY_LEFT_CONTROL])
        engine->moveCamera({0.0f, -moveSpeed * dt, 0.0f});*/

    if (engine->input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT])
    {
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
    }
    else
    {
        engine->enableCursor();
        firstMouse = true;
    }

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

void tryAddChunk(int dx, int dz, glm::ivec3 playerChunk)
{
    glm::ivec3 chunkPos = playerChunk + glm::ivec3(dx * ChunkData::chunkSize, 0, dz * ChunkData::chunkSize);

    if (!world.chunkLoadQueue.has(chunkPos) && !world.hasChunk(chunkPos))
    {
        world.chunkLoadQueue.push(chunkPos);
    }
}

void update(Engine *engine, float dt)
{
    if (engine->input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT])
    {
        glm::vec3 movementInput(0.0f);

        float moveSpeed = 2.0f;
        if (engine->input.keys[GLFW_KEY_LEFT_SHIFT])
            moveSpeed *= 2.0f;

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
            playerVel.y += 18 * dt;

        movementInput.y = 0.0f;

        if (glm::length(movementInput) > 0.0f)
            movementInput = glm::normalize(movementInput);

        playerVel.x = movementInput.x * moveSpeed;
        playerVel.z = movementInput.z * moveSpeed;

        playerVel.y -= 9 * dt;
        playerPos = playerPos + playerVel * dt;
        resolveCollisions(playerPos, playerVel, glm::vec3(0.8f, 1.5f, 0.8f));
        engine->gameObjects.at("player").position = playerPos;
    }
    updateCam(engine, dt);

    glm::ivec3 playerChunk = world.worldToChunkCoords(engine->camera.Position.x, 0, engine->camera.Position.z);

    int renderDistance = 8;

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
        engine->createGameObject(identifier, chunkPos, glm::vec3(0), glm::vec3(1));

        MaterialData ground;
        ground.diffuseColor = {0.5, 0.5, 0.5};
        ground.hasTexture = 1;

        engine->addVoxelMeshToObject(identifier, ground, "textures/tiles.png", vertices, indices);

        uploads++;
    }
}

int main()
{
    Engine engine;
    try
    {
        engine.init("Game Engine", start, update);

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
        /*
                        engine.createGameObject("cube 1", glm::vec3(0, 25, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
                        engine.addMeshToObject("cube 1", ground, "textures/wood.png", cubeVertices, cubeIndices);

                        engine.createGameObject("cube 2", glm::vec3(10, 30, 0), glm::vec3(45, 45, 45), glm::vec3(1, 1, 1));
                        engine.addMeshToObject("cube 2", ground, "textures/wood.png", cubeVertices, cubeIndices);

                        engine.createGameObject("couch", glm::vec3(30, 30, 0), glm::vec3(45, 45, 45), glm::vec3(0.1f, 0.1f, 0.1f));
                        engine.loadModel("couch", "models/couch/couch1.obj", "models/couch/couch1.mtl");*/

        /*world.loadChunk(glm::ivec3(0, 0, 0));
        world.loadChunk(glm::ivec3(16, 0, 0));
        world.loadChunk(glm::ivec3(0, 0, 16));
        world.loadChunk(glm::ivec3(16, 0, 16));
        world.loadChunk(glm::ivec3(32, 0, 0));
        world.loadChunk(glm::ivec3(32, 0, 16));
        world.loadChunk(glm::ivec3(16, 0, 32));
        world.loadChunk(glm::ivec3(0, 0, 32));
        world.loadChunk(glm::ivec3(32, 0, 32));
        world.renderChunk(&engine, "chunk1", glm::ivec3(0, 0, 0));
        world.renderChunk(&engine, "chunk2", glm::ivec3(16, 0, 0));
        world.renderChunk(&engine, "chunk3", glm::ivec3(0, 0, 16));
        world.renderChunk(&engine, "chunk4", glm::ivec3(16, 0, 16));
        world.renderChunk(&engine, "chunk5", glm::ivec3(32, 0, 0));
        world.renderChunk(&engine, "chunk6", glm::ivec3(32, 0, 16));
        world.renderChunk(&engine, "chunk7", glm::ivec3(16, 0, 32));
        world.renderChunk(&engine, "chunk8", glm::ivec3(0, 0, 32));
        world.renderChunk(&engine, "chunk9", glm::ivec3(32, 0, 32));*/
        world.startWorker();
        /*
        world.startWorker();
        world.startWorker();
        world.startWorker();
        world.startWorker();
        world.startWorker();
        world.startWorker();*/
        engine.run();
        engine.shutdown();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
