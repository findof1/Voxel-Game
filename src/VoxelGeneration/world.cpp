#include "world.hpp"
#include "engine.hpp"
#include <cstdlib>

BlockType World::getBlock(int x, int y, int z) const
{
  glm::ivec3 chunkPos = worldToChunkCoords(x, y, z);
  glm::ivec3 localPos = worldToLocalCoords(x, y, z);

  auto it = chunks.find(chunkPos);
  if (it == chunks.end())
    return BlockType::Nothing;

  return it->second->getBlock(localPos.x, localPos.y, localPos.z);
}

void World::setBlock(int x, int y, int z, BlockType type)
{
  glm::ivec3 chunkPos = worldToChunkCoords(x, y, z);
  glm::ivec3 localPos = worldToLocalCoords(x, y, z);

  auto it = chunks.find(chunkPos);
  if (it == chunks.end())
    return;

  it->second->setBlock(localPos.x, localPos.y, localPos.z, type);
}

void World::loadChunk(const glm::ivec3 &chunkPos)
{
  // check if chunk is stored in disk
  // if so, then load
  // else, generate new chunk
  if (!hasChunk(chunkPos))
  {
    generateChunk(chunkPos);
  }
}

bool World::hasChunk(const glm::ivec3 &chunkPos) const
{
  return chunks.find(chunkPos) != chunks.end();
}

void World::unloadChunk(const glm::ivec3 &chunkPos)
{
  // store chunk in disk

  chunks.erase(chunkPos);
}

int floorDivide(int numerator, int denominator)
{
  if (numerator >= 0)
  {
    return numerator / denominator;
  }

  return (numerator - denominator + 1) / denominator;
}

glm::ivec3 World::worldToChunkCoords(int x, int y, int z) const
{
  return {
      floorDivide(x, ChunkData::chunkSize),
      floorDivide(y, ChunkData::chunkHeight),
      floorDivide(z, ChunkData::chunkSize)};
}

glm::ivec3 World::worldToLocalCoords(int x, int y, int z) const
{
  int localX = x % ChunkData::chunkSize;
  if (localX < 0)
    localX += ChunkData::chunkSize;

  int localY = y % ChunkData::chunkHeight;
  if (localY < 0)
    localY += ChunkData::chunkSize;

  int localZ = z % ChunkData::chunkSize;
  if (localZ < 0)
    localZ += ChunkData::chunkSize;

  return {localX, localY, localZ};
}

void World::renderChunk(Engine *engine, std::string identifier, const glm::ivec3 &chunkPos)
{
  loadChunk(chunkPos);
  meshGenerator.generateMesh(chunks.at(chunkPos));
  engine->createGameObject(identifier, chunkPos, glm::vec3(0), glm::vec3(1));

  MaterialData ground;
  ground.diffuseColor = {0.5, 0.5, 0.5};
  ground.hasTexture = 1;
  engine->addMeshToObject(identifier, ground, "textures/wood.png", meshGenerator.vertices, meshGenerator.indices);
}

void World::generateChunk(const glm::ivec3 &chunkPos)
{
  auto chunk = std::make_unique<ChunkData>(glm::vec3(chunkPos));
  int baseHeight = ChunkData::chunkHeight / 2;
  for (int x = 0; x < ChunkData::chunkSize; ++x)
  {
    for (int z = 0; z < ChunkData::chunkSize; ++z)
    {
      int heightOffset = (std::rand() % 3) - 1;
      int finalHeight = baseHeight + heightOffset;
      for (int y = 0; y < ChunkData::chunkHeight; ++y)
      {
        if (y < finalHeight)
          chunk->setBlock(x, y, z, BlockType::Dirt);
        else
          chunk->setBlock(x, y, z, BlockType::Nothing);
      }
    }
  }
  chunks.emplace(chunkPos, std::move(chunk));
}