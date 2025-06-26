#include "world.hpp"
#include "engine.hpp"
#include <cstdlib>
#include <glm/gtc/noise.hpp>

World::World() : textureDataSource(0.1f, 0.1f)
{

  TextureData grassTexture;
  grassTexture.up = glm::vec2(8, 1);
  grassTexture.down = glm::vec2(7, 6);
  grassTexture.side = glm::vec2(7, 5);
  grassTexture.blockType = BlockType::Grass_Dirt;

  TextureData dirtTexture;
  dirtTexture.up = glm::vec2(7, 6);
  dirtTexture.down = glm::vec2(7, 6);
  dirtTexture.side = glm::vec2(7, 6);
  dirtTexture.blockType = BlockType::Dirt;

  TextureData waterTexture;
  waterTexture.up = glm::vec2(0, 7);
  waterTexture.down = glm::vec2(0, 7);
  waterTexture.side = glm::vec2(0, 7);
  waterTexture.blockType = BlockType::Water;

  TextureData sandTexture;
  sandTexture.up = glm::vec2(3, 7);
  sandTexture.down = glm::vec2(3, 7);
  sandTexture.side = glm::vec2(3, 7);
  sandTexture.blockType = BlockType::Sand;

  TextureData stoneTexture;
  stoneTexture.up = glm::vec2(3, 5);
  stoneTexture.down = glm::vec2(3, 5);
  stoneTexture.side = glm::vec2(3, 5);
  stoneTexture.blockType = BlockType::Stone;
  /*
   TextureData treeTrunkTexture;
   treeTrunkTexture.up = glm::vec2(0, 0);
   treeTrunkTexture.down = glm::vec2(0, 0);
   treeTrunkTexture.side = glm::vec2(1, 9);
   treeTrunkTexture.blockType = BlockType::TreeTrunk;

   TextureData birchTreeTrunkTexture;
   birchTreeTrunkTexture.up = glm::vec2(0, 2);
   birchTreeTrunkTexture.down = glm::vec2(0, 2);
   birchTreeTrunkTexture.side = glm::vec2(0, 1);
   birchTreeTrunkTexture.blockType = BlockType::BirchTreeTrunk;

   TextureData birchTreeLeafesSolidTexture;
   birchTreeLeafesSolidTexture.up = glm::vec2(5, 9);
   birchTreeLeafesSolidTexture.down = glm::vec2(5, 9);
   birchTreeLeafesSolidTexture.side = glm::vec2(5, 9);
   birchTreeLeafesSolidTexture.blockType = BlockType::BirchTreeLeafesSolid;

   TextureData treeLeafesSolidTexture;
   treeLeafesSolidTexture.up = glm::vec2(5, 8);
   treeLeafesSolidTexture.down = glm::vec2(5, 8);
   treeLeafesSolidTexture.side = glm::vec2(5, 8);
   treeLeafesSolidTexture.blockType = BlockType::TreeLeafesSolid;

   TextureData grassStoneTexture;
   grassStoneTexture.up = glm::vec2(8, 9);
   grassStoneTexture.down = glm::vec2(3, 5);
   grassStoneTexture.side = glm::vec2(2, 5);
   grassStoneTexture.blockType = BlockType::Grass_Stone;
   */

  textureDataSource.addTextureData(grassTexture, BlockType::Grass_Dirt);
  textureDataSource.addTextureData(dirtTexture, BlockType::Dirt);
  textureDataSource.addTextureData(waterTexture, BlockType::Water);
  textureDataSource.addTextureData(sandTexture, BlockType::Sand);
  textureDataSource.addTextureData(stoneTexture, BlockType::Stone);
  /*
  textureDataSource.textureDataList.push_back(stoneTexture);
  textureDataSource.textureDataList.push_back(treeTrunkTexture);
  textureDataSource.textureDataList.push_back(treeLeafesSolidTexture);
  textureDataSource.textureDataList.push_back(birchTreeTrunkTexture);
  textureDataSource.textureDataList.push_back(birchTreeLeafesSolidTexture);
  textureDataSource.textureDataList.push_back(grassStoneTexture);*/
}

BlockType World::getBlock(int x, int y, int z) const
{
  glm::ivec3 chunkPos = worldToChunkCoords(x, y, z);
  glm::ivec3 localPos = worldToLocalCoords(x, y, z);

  auto it = chunks.find(chunkPos);
  if (it == chunks.end())
  {

    return BlockType::Nothing;
  }

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
      floorDivide(x, ChunkData::chunkSize) * ChunkData::chunkSize,
      floorDivide(y, ChunkData::chunkHeight) * ChunkData::chunkHeight,
      floorDivide(z, ChunkData::chunkSize) * ChunkData::chunkSize};
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
  meshGenerator.generateMesh(this, textureDataSource, chunks.at(chunkPos));
  engine->createGameObject(identifier, chunkPos, glm::vec3(0), glm::vec3(1));

  MaterialData ground;
  ground.diffuseColor = {0.5, 0.5, 0.5};
  ground.hasTexture = 1;
  engine->addMeshToObject(identifier, ground, "textures/tiles.png", meshGenerator.vertices, meshGenerator.indices);
}

void World::generateChunk(const glm::ivec3 &chunkPos)
{
  auto chunk = std::make_unique<ChunkData>(glm::vec3(chunkPos));

  int waterLevel = ChunkData::chunkHeight / 2;
  float scale = 0.1f;

  for (int x = 0; x < ChunkData::chunkSize; ++x)
  {
    for (int z = 0; z < ChunkData::chunkSize; ++z)
    {
      int worldX = chunkPos.x + x;
      int worldZ = chunkPos.z + z;

      float noiseValue = glm::perlin(glm::vec2(worldX, worldZ) * scale);
      noiseValue = (noiseValue + 1.0f) / 2.0f;

      int terrainHeight = static_cast<int>(noiseValue * (ChunkData::chunkHeight * 0.5f)) + (ChunkData::chunkHeight / 4);

      for (int y = 0; y < ChunkData::chunkHeight; ++y)
      {
        if (y < terrainHeight - 3)
        {
          chunk->setBlock(x, y, z, BlockType::Stone);
        }
        else if (y < terrainHeight - 1)
        {
          chunk->setBlock(x, y, z, BlockType::Dirt);
        }
        else if (y < terrainHeight)
        {
          if (terrainHeight <= waterLevel + 1)
            chunk->setBlock(x, y, z, BlockType::Sand);
          else
            chunk->setBlock(x, y, z, BlockType::Grass_Dirt);
        }
        else if (y < waterLevel)
        {
          chunk->setBlock(x, y, z, BlockType::Water);
        }
        else
        {
          chunk->setBlock(x, y, z, BlockType::Air);
        }
      }
    }
  }

  chunks.emplace(chunkPos, std::move(chunk));
}