#include <unordered_map>
#include "chunkData.hpp"
#include <glm/glm.hpp>
#include <memory>
#include "meshGenerator.hpp"
#include "blockDataSO.hpp"
#include "MutlithreadingQueue.hpp"
#include <thread>
#include <random>

struct BlockQueueData
{
  glm::ivec3 blockPos;
  BlockType blockType;
};

struct Biome
{
  std::string name;
  BlockType surfaceBlock;
  BlockType underwaterSurfaceBlock;
  BlockType fillerBlock;
  BlockType undergroundBlock;
  float temp;
  float humidity;
  int elevation;
  int heightDiff;
  float persistance;
  float lacunarity;

  Biome(const char *name,
        BlockType surfaceBlock,
        BlockType underwaterSurfaceBlock,
        BlockType fillerBlock,
        BlockType undergroundBlock,
        float temp,
        float humidity,
        int elevation,
        int heightDiff,
        float persistance,
        float lacunarity)
      : name(name),
        surfaceBlock(surfaceBlock),
        underwaterSurfaceBlock(underwaterSurfaceBlock),
        fillerBlock(fillerBlock),
        undergroundBlock(undergroundBlock),
        temp(temp),
        humidity(humidity),
        elevation(elevation),
        heightDiff(heightDiff),
        persistance(persistance),
        lacunarity(lacunarity)
  {
  }
};

struct ChunkHasher
{
  size_t operator()(const glm::ivec3 &v) const
  {
    return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1) ^ (std::hash<int>()(v.z) << 2);
  }
};

class Engine;
class World
{
public:
  MeshGenerator meshGenerator;
  BlockDataSO textureDataSource;
  std::vector<Biome> biomes;
  std::unordered_map<glm::ivec3, std::shared_ptr<ChunkData>, ChunkHasher> chunks;
  std::unordered_map<glm::ivec3, std::vector<BlockQueueData>, ChunkHasher> blocksQueue;

  World();

  BlockType getBlock(int x, int y, int z) const;

  int setBlock(int x, int y, int z, BlockType type);

  void loadChunk(const glm::ivec3 &chunkPos);

  bool hasChunk(const glm::ivec3 &chunkPos) const;

  void unloadChunk(const glm::ivec3 &chunkPos);

  glm::ivec3 worldToChunkCoords(int x, int y, int z) const;

  glm::ivec3 worldToLocalCoords(int x, int y, int z) const;

  void renderChunk(Engine *engine, std::string identifier, const glm::ivec3 &chunkPos);

  void generateChunk(const glm::ivec3 &chunkPos);

  std::pair<int, BlockType> getSurfaceBlock(std::shared_ptr<ChunkData> chunk, int x, int z)
  {
    for (int y = ChunkData::chunkHeight - 1; y >= 0; --y)
    {
      BlockType type = chunk->getBlock(x, y, z);
      if (type != BlockType::Air && type != BlockType::Nothing)
        return {y, type};
    }
    return {-1, BlockType::Nothing};
  }

  void generateTreeAt(std::shared_ptr<ChunkData> chunk, const glm::ivec3 &position, std::mt19937 &rng);

  std::pair<float, Biome> getBiome(float temperature, float humidity, float elevation);
  std::pair<float, Biome> getBiomeSecondary(float temperature, float humidity, float elevation);

  ChunkQueue chunkLoadQueue;
  CompletedQueue completedMeshes;
  std::atomic<bool> running = true;
  mutable std::mutex chunkMutex;
  mutable std::mutex chunkDeletionMutex;

  void chunkWorker()
  {

    MeshGenerator threadMeshGenerator;
    while (running)
    {
      glm::ivec3 pos;
      if (chunkLoadQueue.pop(pos))
      {
        loadChunk(pos);

        bool needsMeshing = false;
        std::shared_ptr<ChunkData> chunk;
        {
          std::lock_guard<std::mutex> lock(chunkMutex);
          needsMeshing = chunks.find(pos) != chunks.end() && chunks.at(pos)->modifiedChunk;

          if (needsMeshing)
          {
            chunk = chunks.at(pos);
            chunk->modifiedChunk = false;
          }
        }

        if (needsMeshing)
        {
          std::vector<Vertex> vertices;
          std::vector<uint32_t> indices;
          std::lock_guard<std::mutex> lock(chunkDeletionMutex);
          threadMeshGenerator.generateMesh(this, textureDataSource, chunk);

          completedMeshes.push({pos, std::move(threadMeshGenerator.vertices), std::move(threadMeshGenerator.indices)});
        }
      }
      else
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      }
    }
  }
  void startWorker()
  {
    std::thread([this]()
                { this->chunkWorker(); })
        .detach();
  }
};