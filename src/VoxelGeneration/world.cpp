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

  biomes.emplace_back("Plains", BlockType::Grass_Dirt, BlockType::Sand, BlockType::Dirt, BlockType::Stone, 0.4f, 0.6f, 0.5f, 0.5f, 0.3f, 0.8f, 55, 4, 0.01f);
  biomes.emplace_back("Forest", BlockType::Grass_Dirt, BlockType::Sand, BlockType::Dirt, BlockType::Stone, 0.4f, 0.6f, 0.6f, 0.6f, 0.3f, 0.8f, 55, 6, 0.05f);
  biomes.emplace_back("Desert", BlockType::Sand, BlockType::Sand, BlockType::Sand, BlockType::Stone, 0.8f, 0.9f, 0.0f, 0.1f, 0.3f, 0.8f, 55, 4, 0.01f);
  biomes.emplace_back("Ocean", BlockType::Sand, BlockType::Sand, BlockType::Sand, BlockType::Stone, 0.4f, 0.5f, 0.8f, 0.8f, 0.0f, 0.0f, 50, 4, 0.01f);
  biomes.emplace_back("Mountain", BlockType::Stone, BlockType::Sand, BlockType::Stone, BlockType::Stone, 0.3f, 0.3f, 0.2f, 0.8f, 1.0f, 1.0f, 75, 10, 0.09f);
}

BlockType World::getBlock(int x, int y, int z) const
{
  glm::ivec3 chunkPos = worldToChunkCoords(x, y, z);
  glm::ivec3 localPos = worldToLocalCoords(x, y, z);

  std::lock_guard<std::mutex> lock(chunkMutex);
  auto it = chunks.find(chunkPos);
  if (it == chunks.end())
  {
    return BlockType::Nothing;
  }
  assert(it->second != nullptr);
  BlockType block = it->second->getBlock(localPos.x, localPos.y, localPos.z);

  return block;
}

void World::setBlock(int x, int y, int z, BlockType type)
{
  glm::ivec3 chunkPos = worldToChunkCoords(x, y, z);
  glm::ivec3 localPos = worldToLocalCoords(x, y, z);

  auto it = chunks.find(chunkPos);
  if (it == chunks.end())
    return;

  std::lock_guard<std::mutex> lock(chunkMutex);
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

Biome World::getBiome(float temperature, float humidity, float elevation)
{
  float closestDistance = std::numeric_limits<float>::max();
  const Biome *closestBiome = nullptr;

  for (const Biome &biome : biomes)
  {
    float biomeTempCenter = (biome.minTemp + biome.maxTemp) * 0.5f;
    float biomeHumidCenter = (biome.minHumid + biome.maxHumid) * 0.5f;
    float biomeElevCenter = (biome.minElev + biome.maxElev) * 0.5f;

    float distSq = (temperature - biomeTempCenter) * (temperature - biomeTempCenter) + (humidity - biomeHumidCenter) * (humidity - biomeHumidCenter) + (elevation - biomeElevCenter) * (elevation - biomeElevCenter);

    if (distSq < closestDistance)
    {
      closestDistance = distSq;
      closestBiome = &biome;
    }
  }

  return *closestBiome;
}

std::tuple<const Biome &, const Biome &, bool> World::getTwoClosestBiomes(float temperature, float humidity, float elevation)
{
  const Biome *closest;
  const Biome *secondClosest;
  float closestDist = std::numeric_limits<float>::max();
  float secondDist = std::numeric_limits<float>::max();

  for (const Biome &biome : biomes)
  {
    float biomeTempCenter = (biome.minTemp + biome.maxTemp) * 0.5f;
    float biomeHumidCenter = (biome.minHumid + biome.maxHumid) * 0.5f;
    float biomeElevCenter = (biome.minElev + biome.maxElev) * 0.5f;

    float distSq =
        (temperature - biomeTempCenter) * (temperature - biomeTempCenter) +
        (humidity - biomeHumidCenter) * (humidity - biomeHumidCenter) +
        (elevation - biomeElevCenter) * (elevation - biomeElevCenter);

    if (distSq < closestDist)
    {
      secondDist = closestDist;
      secondClosest = closest;

      closestDist = distSq;
      closest = &biome;
    }
    else if (distSq < secondDist)
    {
      secondDist = distSq;
      secondClosest = &biome;
    }
  }

  bool blendBiomes = false;
  if (closestDist - secondDist < 0.1)
  {
    blendBiomes = true;
  }

  return {*closest, *secondClosest, true};
}

void World::generateChunk(const glm::ivec3 &chunkPos)
{
  auto chunk = std::make_unique<ChunkData>(glm::vec3(chunkPos));

  int waterLevel = 50;

  for (int x = 0; x < ChunkData::chunkSize; ++x)
  {
    for (int z = 0; z < ChunkData::chunkSize; ++z)
    {
      int worldX = chunkPos.x + x;
      int worldZ = chunkPos.z + z;
      float temperature = glm::perlin(glm::vec2(worldX + 120000, worldZ + 150000) * 0.001f);
      float humidity = glm::perlin(glm::vec2(worldX + 6000, worldZ + 14000) * 0.001f);
      float elevation = glm::perlin(glm::vec2(worldX + 15000, worldZ + 20000) * 0.005f);
      auto [biome, biome2, blendBiomes] = getTwoClosestBiomes(temperature, humidity, elevation);
      int baseHeight = biome.baseHeight;
      int heightDifference = biome.heightDifference;
      float perlinHeightScale = biome.perlinHeightScale;
      if (blendBiomes)
      {
        baseHeight = (biome.baseHeight + biome2.baseHeight) / 2;
        heightDifference = (biome.heightDifference + biome2.heightDifference) / 2;
        perlinHeightScale = (biome.perlinHeightScale + biome2.perlinHeightScale) / 2;
      }

      float noiseValue = glm::perlin(glm::vec2(worldX, worldZ) * perlinHeightScale);

      int terrainHeight = static_cast<int>(noiseValue * heightDifference) + baseHeight;

      for (int y = 0; y < ChunkData::chunkHeight; ++y)
      {
        if (y < terrainHeight - 3)
        {
          chunk->setBlock(x, y, z, biome.undergroundBlock);
        }
        else if (y < terrainHeight - 1)
        {
          chunk->setBlock(x, y, z, biome.fillerBlock);
        }
        else if (y < terrainHeight)
        {
          if (terrainHeight <= waterLevel + 1)
            chunk->setBlock(x, y, z, biome.underwaterSurfaceBlock);
          else
            chunk->setBlock(x, y, z, biome.surfaceBlock);
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
  std::lock_guard<std::mutex> lock(chunkMutex);
  chunks.emplace(chunkPos, std::move(chunk));
}