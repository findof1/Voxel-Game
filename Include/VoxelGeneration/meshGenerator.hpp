#pragma once
#include "chunkData.hpp"
#include <vertex.h>
#include <memory>
#include "blockDataSO.hpp"

const glm::ivec3 directions[6] = {
    {1, 0, 0},
    {-1, 0, 0},
    {0, 1, 0},
    {0, -1, 0},
    {0, 0, 1},
    {0, 0, -1}};

const glm::vec3 faceVertices[6][4] = {
    // +X
    {{0.5, -0.5, -0.5}, {0.5, 0.5, -0.5}, {0.5, 0.5, 0.5}, {0.5, -0.5, 0.5}},
    // -X
    {{-0.5, -0.5, 0.5}, {-0.5, 0.5, 0.5}, {-0.5, 0.5, -0.5}, {-0.5, -0.5, -0.5}},
    // +Y
    {{-0.5, 0.5, 0.5}, {0.5, 0.5, 0.5}, {0.5, 0.5, -0.5}, {-0.5, 0.5, -0.5}},
    // -Y
    {{-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5}, {0.5, -0.5, 0.5}, {-0.5, -0.5, 0.5}},
    // +Z
    {{-0.5, -0.5, 0.5}, {0.5, -0.5, 0.5}, {0.5, 0.5, 0.5}, {-0.5, 0.5, 0.5}},
    // -Z
    {{0.5, -0.5, -0.5}, {-0.5, -0.5, -0.5}, {-0.5, 0.5, -0.5}, {0.5, 0.5, -0.5}}};

class World;
class MeshGenerator
{
public:
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  bool generateMesh(World *world, BlockDataSO &textureData, const std::shared_ptr<ChunkData> &chunk);
  void greedyMeshDirection(World *world, BlockDataSO &textureData, const std::shared_ptr<ChunkData> &chunk, int face, uint32_t &indexCount);
};