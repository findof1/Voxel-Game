#include "chunkData.hpp"
#include <vertex.h>
#include <memory>

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

class MeshGenerator
{
public:
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  void generateMesh(const std::unique_ptr<ChunkData> &chunk)
  {
    vertices.clear();
    indices.clear();

    uint32_t indexCount = 0;

    for (int x = 0; x < ChunkData::chunkSize; ++x)
      for (int y = 0; y < ChunkData::chunkHeight; ++y)
        for (int z = 0; z < ChunkData::chunkSize; ++z)
        {
          BlockType block = chunk->getBlock(x, y, z);
          if (block == BlockType::Nothing || block == BlockType::Air)
            continue;

          for (int face = 0; face < 6; ++face)
          {
            glm::ivec3 neighborPos = glm::ivec3(x, y, z) + directions[face];
            BlockType neighborBlock;
            if (neighborPos.x < 0 || neighborPos.x >= ChunkData::chunkSize ||
                neighborPos.y < 0 || neighborPos.y >= ChunkData::chunkHeight ||
                neighborPos.z < 0 || neighborPos.z >= ChunkData::chunkSize)
            {
              neighborBlock = BlockType::Nothing;
            }
            else
            {
              neighborBlock = chunk->getBlock(neighborPos.x, neighborPos.y, neighborPos.z);
            }

            if (neighborBlock == BlockType::Nothing || neighborBlock == BlockType::Air)
            {
              for (int vert = 0; vert < 4; ++vert)
              {
                glm::vec3 vertexPos = glm::vec3(x, y, z) + faceVertices[face][vert];
                Vertex vertex;
                vertex.color = glm::vec3(0);
                vertex.normal = directions[face];
                vertex.pos = vertexPos;

                glm::vec2 uv = (vert == 0 || vert == 3) ? glm::vec2(0.0f, 0.0f) : glm::vec2(1.0f, 1.0f);
                vertex.texPos = uv;

                vertices.emplace_back(vertex);
              }

              indices.push_back(indexCount + 0);
              indices.push_back(indexCount + 1);
              indices.push_back(indexCount + 2);

              indices.push_back(indexCount + 2);
              indices.push_back(indexCount + 3);
              indices.push_back(indexCount + 0);

              indexCount += 4;
            }
          }
        }
  }
};