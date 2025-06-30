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

void start(Engine *engine)
{
}

World world;

void update(Engine *engine, float dt)
{

    glm::ivec3 playerChunk = world.worldToChunkCoords(engine->camera.Position.x, 0, engine->camera.Position.z);

    std::cout << engine->camera.Position.x << "|" << engine->camera.Position.z << std::endl;
    int renderDistance = 8;

    for (int x = -renderDistance; x <= renderDistance; ++x)
    {
        for (int z = -renderDistance; z <= renderDistance; ++z)
        {
            glm::ivec3 chunkPos = playerChunk + glm::ivec3(x * ChunkData::chunkSize, 0, z * ChunkData::chunkSize);

            if (!world.chunkLoadQueue.has(chunkPos))
            {
                world.chunkLoadQueue.push(chunkPos);
            }
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
        engine.useFreeCamMode(true);

        MaterialData ground;
        ground.diffuseColor = {0.5, 0.5, 0.5};
        ground.hasTexture = 1;

        engine.createGameObject("skyBox", glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1000, 1000, 1000));
        engine.addMeshToObject("skyBox", ground, "textures/sky.png", cubeVerticesNoNormals, skyBoxIndices);
        /*
                engine.createGameObject("object", glm::vec3(0, -10, 0), glm::vec3(0, 0, 0), glm::vec3(100, 25, 100));
                engine.addMeshToObject("object", ground, "textures/wood.png", cubeVertices, cubeIndices);

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
        world.startWorker();
        world.startWorker();
        world.startWorker();
        world.startWorker();
        world.startWorker();
        world.startWorker();
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
