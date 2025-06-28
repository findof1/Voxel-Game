#include "meshGenerator.hpp"
#include "world.hpp"

bool MeshGenerator::generateMesh(World *world, BlockDataSO &textureData, const std::shared_ptr<ChunkData> &chunk)
{
  vertices.clear();
  indices.clear();

  uint32_t indexCount = 0;

  for (int x = 0; x < ChunkData::chunkSize; ++x)
    for (int y = 0; y < ChunkData::chunkHeight; ++y)
      for (int z = 0; z < ChunkData::chunkSize; ++z)
      {
        BlockType block;
        {
          std::lock_guard<std::mutex> lock(world->chunkMutex);
          block = chunk->getBlock(x, y, z);
        }
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
            glm::ivec3 worldPosition = chunk->worldPosition + neighborPos;

            neighborBlock = world->getBlock(worldPosition.x, worldPosition.y, worldPosition.z); // has mutex lock inside btw
          }
          else
          {
            std::lock_guard<std::mutex> lock(world->chunkMutex);
            neighborBlock = chunk->getBlock(neighborPos.x, neighborPos.y, neighborPos.z);
          }

          if (neighborBlock == BlockType::Nothing || neighborBlock == BlockType::Air || (block != BlockType::Water && neighborBlock == BlockType::Water))
          {
            for (int vert = 0; vert < 4; ++vert)
            {
              glm::vec3 vertexPos = glm::vec3(x, y, z) + faceVertices[face][vert];
              Vertex vertex;
              vertex.color = glm::vec3(0);
              vertex.normal = directions[face];
              vertex.pos = vertexPos;

              glm::vec2 uv;

              if (face == 2)
              {
                uv = glm::vec2(textureData.textureDataList.at(block).up.x * textureData.textureSizeX, textureData.textureDataList.at(block).up.y * textureData.textureSizeY);
              }
              else if (face == 3)
              {
                uv = glm::vec2(textureData.textureDataList.at(block).down.x * textureData.textureSizeX, textureData.textureDataList.at(block).down.y * textureData.textureSizeY);
              }
              else
              {
                uv = glm::vec2(textureData.textureDataList.at(block).side.x * textureData.textureSizeX, textureData.textureDataList.at(block).side.y * textureData.textureSizeY);
              }

              // This works I guess. Don't know why, but it does...
              if (face == 2 || face == 3 || face == 0 || face == 1)
              {
                if (vert == 1)
                {
                  uv.y -= textureData.textureSizeY;
                }
                if (vert == 2)
                {
                  uv.y -= textureData.textureSizeY;
                  uv.x += textureData.textureSizeX;
                }
                if (vert == 3)
                {
                  uv.x += textureData.textureSizeX;
                }
              }
              else if (face == 4 || face == 5)
              {
                if (vert == 1)
                {
                  uv.x += textureData.textureSizeX;
                }
                if (vert == 2)
                {
                  uv.y -= textureData.textureSizeY;
                  uv.x += textureData.textureSizeX;
                }
                if (vert == 3)
                {
                  uv.y -= textureData.textureSizeY;
                }
              }
              vertex.texPos = uv;
              //

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
  return true;
}