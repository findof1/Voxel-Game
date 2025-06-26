#include <unordered_map>
#include "chunkData.hpp"
#include <glm/glm.hpp>
#include <memory>
#include "meshGenerator.hpp"

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
  BlockType getBlock(int x, int y, int z) const;

  void setBlock(int x, int y, int z, BlockType type);

  void loadChunk(const glm::ivec3 &chunkPos);

  bool hasChunk(const glm::ivec3 &chunkPos) const;

  void unloadChunk(const glm::ivec3 &chunkPos);

  glm::ivec3 worldToChunkCoords(int x, int y, int z) const;

  glm::ivec3 worldToLocalCoords(int x, int y, int z) const;

  void renderChunk(Engine *engine, std::string identifier, const glm::ivec3 &chunkPos);

  void generateChunk(const glm::ivec3 &chunkPos);

private:
  std::unordered_map<glm::ivec3, std::unique_ptr<ChunkData>, ChunkHasher> chunks;
};