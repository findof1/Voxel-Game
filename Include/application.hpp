#pragma once
#include "engine.hpp"
#include "primitives.hpp"
#include <world.hpp>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class Item
{
  None,
  Grass_Dirt,
  Dirt,
  Stone,
  Sand,
  Tree_Trunk,
  Tree_Leafes_Solid
};

struct ItemInfo
{
  std::string displayName;
  int maxStackSize;
};

const std::unordered_map<Item, ItemInfo> itemDatabase = {
    {Item::None, {"", 0}},
    {Item::Grass_Dirt, {"Grass", 64}},
    {Item::Dirt, {"Dirt", 64}},
    {Item::Stone, {"Stone", 32}},
    {Item::Sand, {"Sand", 64}},
    {Item::Tree_Trunk, {"Wood Log", 16}},
    {Item::Tree_Leafes_Solid, {"Leaves", 64}},
};

struct InventoryItem
{
  Item item;
  bool makeTexture = false;
  int currentStackSize;
};

struct Inventory
{
  std::array<InventoryItem, 10> items;
  int selectedSlot;

  Inventory()
  {
    selectedSlot = 0;
    for (auto &slot : items)
    {
      slot.item = Item::None;
      slot.currentStackSize = 0;
    }
  }
};

inline Item blockToItem(BlockType block)
{
  switch (block)
  {
  case BlockType::Grass_Dirt:
    return Item::Grass_Dirt;
  case BlockType::Dirt:
    return Item::Dirt;
  case BlockType::Stone:
    return Item::Stone;
  case BlockType::Sand:
    return Item::Sand;
  case BlockType::Tree_Trunk:
    return Item::Tree_Trunk;
  case BlockType::Tree_Leafes_Solid:
    return Item::Tree_Leafes_Solid;

  default:
    return Item::None;
  }
}
inline BlockType itemToBlock(Item item)
{
  switch (item)
  {
  case Item::Grass_Dirt:
    return BlockType::Grass_Dirt;
  case Item::Dirt:
    return BlockType::Dirt;
  case Item::Stone:
    return BlockType::Stone;
  case Item::Sand:
    return BlockType::Sand;
  case Item::Tree_Trunk:
    return BlockType::Tree_Trunk;
  case Item::Tree_Leafes_Solid:
    return BlockType::Tree_Leafes_Solid;

  default:
    return BlockType::Air;
  }
}

inline bool addItemToInventory(Inventory &inventory, Item item, int amount)
{
  int maxStackSize = itemDatabase.at(item).maxStackSize;

  for (auto &slot : inventory.items)
  {
    if (slot.currentStackSize > 0 && slot.item == item)
    {
      int space = maxStackSize - slot.currentStackSize;
      int toAdd = std::min(space, amount);
      slot.currentStackSize += toAdd;
      amount -= toAdd;
      if (amount == 0)
        return true;
    }
  }

  for (auto &slot : inventory.items)
  {
    if (slot.currentStackSize == 0 || slot.item == Item::None)
    {
      int toAdd = std::min(maxStackSize, amount);
      slot.item = item;
      slot.makeTexture = true;
      slot.currentStackSize = toAdd;
      amount -= toAdd;
      if (amount == 0)
        return true;
    }
  }

  return false;
}

class Application
{
public:
  void run();

private:
  glm::vec3 playerPos = glm::vec3(0, 255, 0);
  glm::vec3 playerVel = glm::vec3(0);
  Inventory inventory;

  World world;
  bool leftMouseClick = false;
  bool rightMouseClick = false;
  bool spaceClick = false;
  Engine engine;

  bool AABBIntersect(glm::vec3 minA, glm::vec3 maxA, glm::vec3 minB, glm::vec3 maxB);

  glm::vec3 getPenetration(glm::vec3 minA, glm::vec3 maxA, glm::vec3 minB, glm::vec3 maxB);
  void resolveCollisions(glm::vec3 &position, glm::vec3 &velocity, const glm::vec3 &size);

  void start(Engine *engine)
  {
  }

  void updateCam(Engine *engine, float dt);

  void tryAddChunk(int dx, int dz, glm::ivec3 playerChunk);

  bool raycastVoxel(glm::vec3 originInitial, glm::vec3 direction, float maxDistance, glm::ivec3 &hitBlock, glm::ivec3 &hitNormal, BlockType &blockType);

  void update(Engine *engine, float dt);

  void drawInventory();
};