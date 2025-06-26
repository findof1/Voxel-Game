#include "chunkData.hpp"

ChunkData::ChunkData(glm::ivec3 worldPosition) : worldPosition(worldPosition)
{
  std::fill(std::begin(blocks), std::end(blocks), BlockType::Nothing);
}