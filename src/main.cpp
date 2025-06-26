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

void start(Engine *engine)
{
}

void update(Engine *engine, float dt)
{
}

int main()
{
    Engine engine;
    World world;
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

        world.renderChunk(&engine, "chunk1", glm::ivec3(0, 0, 0));

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
