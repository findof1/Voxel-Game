#include "world.hpp"
#include "engine.hpp"
#include <cstdlib>
#include <glm/gtc/noise.hpp>

World::World() : textureDataSource(0.1f, 0.1f)
{
  TextureData grassTexture;
  grassTexture.up = glm::vec2(0, 2);
  grassTexture.down = glm::vec2(1, 1);
  grassTexture.side = glm::vec2(0, 1);
  grassTexture.blockType = BlockType::Grass_Dirt;

  TextureData dirtTexture;
  dirtTexture.up = glm::vec2(1, 1);
  dirtTexture.down = glm::vec2(1, 1);
  dirtTexture.side = glm::vec2(1, 1);
  dirtTexture.blockType = BlockType::Dirt;

  TextureData waterTexture;
  waterTexture.up = glm::vec2(4, 0);
  waterTexture.down = glm::vec2(4, 0);
  waterTexture.side = glm::vec2(4, 0);
  waterTexture.blockType = BlockType::Water;

  TextureData sandTexture;
  sandTexture.up = glm::vec2(3, 0);
  sandTexture.down = glm::vec2(3, 0);
  sandTexture.side = glm::vec2(3, 0);
  sandTexture.blockType = BlockType::Sand;

  TextureData stoneTexture;
  stoneTexture.up = glm::vec2(2, 0);
  stoneTexture.down = glm::vec2(2, 0);
  stoneTexture.side = glm::vec2(2, 0);
  stoneTexture.blockType = BlockType::Stone;

  TextureData treeTrunkTexture;
  treeTrunkTexture.up = glm::vec2(6, 0);
  treeTrunkTexture.down = glm::vec2(6, 0);
  treeTrunkTexture.side = glm::vec2(5, 0);
  treeTrunkTexture.blockType = BlockType::Tree_Trunk;

  TextureData treeLeafesSolidTexture;
  treeLeafesSolidTexture.up = glm::vec2(7, 0);
  treeLeafesSolidTexture.down = glm::vec2(7, 0);
  treeLeafesSolidTexture.side = glm::vec2(7, 0);
  treeLeafesSolidTexture.blockType = BlockType::Tree_Leafes_Solid;

  /*
  TextureData grassTexture;
  grassTexture.up = glm::vec2(8, 0);
  grassTexture.down = glm::vec2(7, 5);
  grassTexture.side = glm::vec2(7, 4);
  grassTexture.blockType = BlockType::Grass_Dirt;

  TextureData dirtTexture;
  dirtTexture.up = glm::vec2(7, 5);
  dirtTexture.down = glm::vec2(7, 5);
  dirtTexture.side = glm::vec2(7, 5);
  dirtTexture.blockType = BlockType::Dirt;

  TextureData waterTexture;
  waterTexture.up = glm::vec2(0, 6);
  waterTexture.down = glm::vec2(0, 6);
  waterTexture.side = glm::vec2(0, 6);
  waterTexture.blockType = BlockType::Water;

  TextureData sandTexture;
  sandTexture.up = glm::vec2(3, 6);
  sandTexture.down = glm::vec2(3, 6);
  sandTexture.side = glm::vec2(3, 6);
  sandTexture.blockType = BlockType::Sand;

  TextureData stoneTexture;
  stoneTexture.up = glm::vec2(3, 4);
  stoneTexture.down = glm::vec2(3, 4);
  stoneTexture.side = glm::vec2(3, 4);
  stoneTexture.blockType = BlockType::Stone;

  TextureData treeTrunkTexture;
  treeTrunkTexture.up = glm::vec2(0, 9);
  treeTrunkTexture.down = glm::vec2(0, 9);
  treeTrunkTexture.side = glm::vec2(1, 0);
  treeTrunkTexture.blockType = BlockType::Tree_Trunk;

  TextureData treeLeafesSolidTexture;
  treeLeafesSolidTexture.up = glm::vec2(5, 1);
  treeLeafesSolidTexture.down = glm::vec2(5, 1);
  treeLeafesSolidTexture.side = glm::vec2(5, 1);
  treeLeafesSolidTexture.blockType = BlockType::Tree_Leafes_Solid;
  */

  textureDataSource.addTextureData(grassTexture, BlockType::Grass_Dirt);
  textureDataSource.addTextureData(dirtTexture, BlockType::Dirt);
  textureDataSource.addTextureData(waterTexture, BlockType::Water);
  textureDataSource.addTextureData(sandTexture, BlockType::Sand);
  textureDataSource.addTextureData(stoneTexture, BlockType::Stone);
  textureDataSource.addTextureData(treeTrunkTexture, BlockType::Tree_Trunk);
  textureDataSource.addTextureData(treeLeafesSolidTexture, BlockType::Tree_Leafes_Solid);

  biomes.emplace_back("Null", BlockType::Nothing, BlockType::Nothing, BlockType::Nothing, BlockType::Nothing, -1, -1, -1, 0, 0, 0);
  biomes.emplace_back("Plains", BlockType::Grass_Dirt, BlockType::Sand, BlockType::Dirt, BlockType::Stone, 0.5f, 0.5f, 0.5f, 10, 0.4f, 1.2f);
  biomes.emplace_back("Forest", BlockType::Grass_Dirt, BlockType::Sand, BlockType::Dirt, BlockType::Stone, 0.6f, 0.7f, 0.7f, 25, 0.5f, 2.0f);
  biomes.emplace_back("Desert", BlockType::Sand, BlockType::Sand, BlockType::Sand, BlockType::Stone, 0.7f, 0.3f, 0.4f, 15, 0.4f, 1.75f);
  biomes.emplace_back("Ocean", BlockType::Sand, BlockType::Sand, BlockType::Sand, BlockType::Stone, 0.5f, 0.6f, 0.2f, 10, 0.4f, 1.2f);
  biomes.emplace_back("Mountain", BlockType::Stone, BlockType::Sand, BlockType::Stone, BlockType::Stone, 0.3f, 0.2f, 0.9f, 90, 0.7f, 2.0f);
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

int World::setBlock(int x, int y, int z, BlockType type)
{
  glm::ivec3 chunkPos = worldToChunkCoords(x, y, z);
  glm::ivec3 localPos = worldToLocalCoords(x, y, z);

  auto it = chunks.find(chunkPos);
  if (it == chunks.end())
    return -1;

  std::lock_guard<std::mutex> lock(chunkMutex);
  return it->second->setBlock(localPos.x, localPos.y, localPos.z, type);
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

std::pair<float, Biome> World::getBiome(float temperature, float humidity, float elevation)
{
  float closestDistance = std::numeric_limits<float>::max();
  const Biome *closestBiome = nullptr;

  for (const Biome &biome : biomes)
  {

    float distSq = (temperature - biome.temp) * (temperature - biome.temp) + (humidity - biome.humidity) * (humidity - biome.humidity) + ((elevation - biome.elevation) * (elevation - biome.elevation));

    if (distSq < closestDistance)
    {
      closestDistance = distSq;
      closestBiome = &biome;
    }
  }

  return {closestDistance, *closestBiome};
}

std::pair<float, Biome> World::getBiomeSecondary(float temperature, float humidity, float elevation)
{
  float closestDistance = std::numeric_limits<float>::max();
  float secondClosestDistance = std::numeric_limits<float>::max();
  const Biome *closestBiome = nullptr;
  const Biome *secondClosestBiome = nullptr;

  for (const Biome &biome : biomes)
  {

    float distSq = (temperature - biome.temp) * (temperature - biome.temp) + (humidity - biome.humidity) * (humidity - biome.humidity) + (elevation - biome.elevation) * (elevation - biome.elevation);

    if (distSq < secondClosestDistance)
    {
      secondClosestDistance = closestDistance;
      secondClosestBiome = closestBiome;

      closestDistance = distSq;
      closestBiome = &biome;
    }
    else if (distSq < closestDistance)
    {
      secondClosestDistance = distSq;
      secondClosestBiome = &biome;
    }
  }

  return {secondClosestDistance, *secondClosestBiome};
}

float getPerlinNoise(float x, float y, int octaves, float persistence, float lacunarity, float scale, float amplitude = 1.0f)
{
  float frequency = 1.0f;
  float noiseHeight = 0.0f;

  for (int i = 0; i < octaves; i++)
  {
    float angle = i * 0.5f;
    float cosA = cos(angle);
    float sinA = sin(angle);

    float sampleX = (x * cosA - y * sinA) * frequency / scale;
    float sampleY = (x * sinA + y * cosA) * frequency / scale;

    float perlinValue = glm::perlin(glm::vec2(sampleX, sampleY));
    noiseHeight += perlinValue * amplitude;

    amplitude *= persistence;
    frequency *= lacunarity;
  }

  return noiseHeight;
}

float edgeBias(float x, float strength = 2.0f)
{
  return pow(x, strength) / (pow(x, strength) + pow(1.0f - x, strength));
}

void World::generateChunk(const glm::ivec3 &chunkPos)
{
  auto chunk = std::make_shared<ChunkData>(glm::vec3(chunkPos));

  int waterLevel = 60;

  bool forestChunk = false;
  for (int x = 0; x < ChunkData::chunkSize; ++x)
  {
    for (int z = 0; z < ChunkData::chunkSize; ++z)
    {
      int worldX = chunkPos.x + x;
      int worldZ = chunkPos.z + z;
      /*
      float temperature = (glm::perlin(glm::vec2(worldX + 120000, worldZ + 150000) * 0.005f) + 1) / 2;
      float humidity = (glm::perlin(glm::vec2(worldX + 8000, worldZ + 12000) * 0.001f) + 1) / 2;
      float elevation = (glm::perlin(glm::vec2(worldX + 10200, worldZ + 18000) * 0.006f) + 1) / 2;
      // std::cout << "Temp: " + std::to_string(temperature) + " Hum: " + std::to_string(humidity) + " Elev: " + std::to_string(elevation) + "\n";

      auto [distance, biome] = getBiome(temperature, humidity, elevation);
      auto [distance2, biome2] = getBiomeSecondary(temperature, humidity, elevation);
      float blendFactor = std::clamp((distance / (distance + distance2)), 0.0f, 1.0f);
      // blendFactor = blendFactor * blendFactor * (3 - 2 * blendFactor);

      int terrainHeight;
      // float noiseValue = getPerlinNoise(worldX, worldZ, 8, 0.7f, 2.0f, 400);
      float noiseValue = getPerlinNoise(worldX, worldZ, 8, glm::mix(biome.persistance, biome2.persistance, blendFactor), glm::mix(biome.lacunarity, biome2.lacunarity, blendFactor), 400);
      terrainHeight = (elevation * 80 + 40) + static_cast<int>(noiseValue * glm::mix(biome.heightDiff, biome2.heightDiff, blendFactor));
*/
      float temperature = (glm::perlin(glm::vec2(worldX + 140000, worldZ + 150000) * 0.005f) + 1) / 2;
      float humidity = (glm::perlin(glm::vec2(worldX + 2000, worldZ + 12000) * 0.001f) + 1) / 2;
      float elevation = (glm::perlin(glm::vec2(worldX + 20200, worldZ + 18000) * 0.006f) + 1) / 2;
      auto [distance, biome] = getBiome(temperature, humidity, elevation);
      if (biome.name == "Forest")
      {
        forestChunk = true;
      }
      int terrainHeight;
      // float noiseValue = getPerlinNoise(worldX, worldZ, 8, 0.7f, 2.0f, 400);
      float persistance = std::pow((getPerlinNoise(worldX + 10000, worldZ, 1, 1.0f, 1.0f, 170) + 1) / 2, 2) * 0.3f + 0.4f;
      float scale = (getPerlinNoise(worldX + 20200, worldZ + 2000, 1, 1.0f, 1.0f, 400) + 1) / 2 * 300 + 300;
      float base = (getPerlinNoise(worldX + 1000, worldZ + 2000, 1, 1.0f, 1.0f, 1800) + 1) / 2 * 120 + 60;

      float amplitude = edgeBias((getPerlinNoise(worldX + 1000, worldZ + 20000, 1, 1.0f, 1.0f, 600) + 1) / 2, 3) * 120 + 5;
      float noiseValue = getPerlinNoise(worldX, worldZ, 8, persistance, 2.0f, scale, amplitude);
      terrainHeight = static_cast<int>(base + noiseValue);
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

  for (BlockQueueData &block : blocksQueue[chunkPos])
  {
    chunk->setBlock(block.blockPos.x, block.blockPos.y, block.blockPos.z, block.blockType);
  }

  if (forestChunk)
  {
    std::mt19937 rng(chunkPos.x * 73856093 ^ chunkPos.z * 19349663);
    std::uniform_int_distribution<int> dist(0, ChunkData::chunkSize - 1);
    for (int i = 0; i < ChunkData::chunkSize / 3; ++i)
    {
      int x = dist(rng);
      int z = dist(rng);

      auto [y, type] = getSurfaceBlock(chunk, x, z);
      if (y == -1)
        continue;
      if (type != BlockType::Grass_Dirt)
        continue;

      generateTreeAt(chunk, {x, y + 1, z}, rng);
    }
  }

  std::lock_guard<std::mutex> lock(chunkMutex);
  chunks.emplace(chunkPos, std::move(chunk));
}

void World::generateTreeAt(std::shared_ptr<ChunkData> chunk, const glm::ivec3 &position, std::mt19937 &rng)
{

  std::uniform_int_distribution<int> trunkHeightDist(4, 40);
  int trunkHeight = trunkHeightDist(rng);
  int trunkWidth = 1;

  if (trunkHeight > 11)
  {
    std::uniform_int_distribution<int> trunkWidthDist(1, trunkHeight / 11);
    trunkWidth = trunkWidthDist(rng);
  }
  std::uniform_int_distribution<int> leafRadiusDist(3, 4 * trunkWidth);
  int leafRadius = leafRadiusDist(rng);

  for (int i = 0; i < trunkHeight; i++)
  {
    for (int w = 0; w < trunkWidth; w++)
    {
      for (int l = 0; l < trunkWidth; l++)
      {
        int result = chunk->setBlock(position.x + w, position.y + i, position.z + l, BlockType::Tree_Trunk);
        if (result == -1)
        {
          glm::vec3 pos = chunk->worldPosition + glm::ivec3(position.x + w, position.y + i, position.z + l);
          result = setBlock(pos.x, pos.y, pos.z, BlockType::Tree_Trunk);
          if (result == -1)
          {
            BlockQueueData block;
            block.blockPos = worldToLocalCoords(pos.x, pos.y, pos.z);
            block.blockType = BlockType::Tree_Trunk;

            blocksQueue[worldToChunkCoords(pos.x, pos.y, pos.z)].emplace_back(block);
          }
        }
      }
    }
  }

  int leafBaseY = position.y + trunkHeight - 2;
  for (int dx = -leafRadius; dx <= leafRadius; ++dx)
  {
    for (int dy = -leafRadius; dy <= leafRadius; ++dy)
    {
      for (int dz = -leafRadius; dz <= leafRadius; ++dz)
      {
        int dist = dx * dx + dy * dy + dz * dz;
        if (dist < leafRadius * leafRadius)
        {
          int result = chunk->setBlock(position.x + dx, leafBaseY + dy, position.z + dz, BlockType::Tree_Leafes_Solid);
          if (result == -1)
          {
            glm::vec3 pos = chunk->worldPosition + glm::ivec3(position.x + dx, leafBaseY + dy, position.z + dz);
            result = setBlock(pos.x, pos.y, pos.z, BlockType::Tree_Leafes_Solid);
            if (result == -1)
            {
              BlockQueueData block;
              block.blockPos = worldToLocalCoords(pos.x, pos.y, pos.z);
              block.blockType = BlockType::Tree_Leafes_Solid;

              blocksQueue[worldToChunkCoords(pos.x, pos.y, pos.z)].emplace_back(block);
            }
          }
        }
      }
    }
  }
}