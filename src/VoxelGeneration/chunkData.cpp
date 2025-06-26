#include "chunkData.hpp"

ChunkData::ChunkData(glm::vec3 worldPosition) : worldPosition(worldPosition)
{
  std::fill(std::begin(blocks), std::end(blocks), BlockType::Nothing);
}