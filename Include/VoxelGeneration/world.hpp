#include <unordered_map>
#include "chunkData.hpp"
#include <glm/glm.hpp>
#include <memory>
#include "meshGenerator.hpp"
#include "blockDataSO.hpp"
#include "MutlithreadingQueue.hpp"
#include <thread>

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
  std::unordered_map<glm::ivec3, std::unique_ptr<ChunkData>, ChunkHasher> chunks;
  World();

  BlockType getBlock(int x, int y, int z) const;

  void setBlock(int x, int y, int z, BlockType type);

  void loadChunk(const glm::ivec3 &chunkPos);

  bool hasChunk(const glm::ivec3 &chunkPos) const;

  void unloadChunk(const glm::ivec3 &chunkPos);

  glm::ivec3 worldToChunkCoords(int x, int y, int z) const;

  glm::ivec3 worldToLocalCoords(int x, int y, int z) const;

  void renderChunk(Engine *engine, std::string identifier, const glm::ivec3 &chunkPos);

  void generateChunk(const glm::ivec3 &chunkPos);

  ChunkQueue chunkLoadQueue;
  CompletedQueue completedMeshes;
  std::atomic<bool> running = true;
  std::mutex chunkMutex;

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
        {
          std::lock_guard<std::mutex> lock(chunkMutex);
          needsMeshing = chunks.find(pos) != chunks.end() && chunks.at(pos)->modifiedChunk;
        }

        if (needsMeshing)
        {
          std::vector<Vertex> vertices;
          std::vector<uint32_t> indices;
          threadMeshGenerator.generateMesh(this, textureDataSource, chunks.at(pos));
          {

            std::lock_guard<std::mutex> lock(chunkMutex);
            chunks.at(pos)->modifiedChunk = false;
          }

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