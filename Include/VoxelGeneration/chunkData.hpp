#ifndef CHUNK_DATA_H
#define CHUNK_DATA_H
#include "blockType.hpp"
#include <vector>
#include <glm/glm.hpp>

class World;
class ChunkData
{
public:
  static const int chunkSize = 16;
  static const int chunkHeight = 256;
  BlockType blocks[chunkSize * chunkSize * chunkHeight];

  glm::ivec3 worldPosition;
  bool modifiedChunk = true;
  ChunkData(const ChunkData &other) = default;
  ChunkData(ChunkData &&other) noexcept = default;
  ChunkData &operator=(const ChunkData &other) = default;
  ChunkData &operator=(ChunkData &&other) noexcept = default;
  ChunkData(glm::ivec3 worldPosition);

  inline int toIndex(int x, int y, int z) const
  {
    return x + chunkSize * (y + chunkHeight * z);
  }

  BlockType getBlock(int x, int y, int z) const
  {
    if (x < 0 || x >= chunkSize || y < 0 || y >= chunkHeight || z < 0 || z >= chunkSize)
      return BlockType::Nothing;

    int idx = toIndex(x, y, z);
    return blocks[idx];
  }

  int setBlock(int x, int y, int z, BlockType type)
  {
    if (x < 0 || x >= chunkSize || y < 0 || y >= chunkHeight || z < 0 || z >= chunkSize)
      return -1;
    blocks[toIndex(x, y, z)] = type;
    modifiedChunk = true;
    return 0;
  }
};

#endif