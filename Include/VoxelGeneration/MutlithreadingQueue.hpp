#pragma once
#include <queue>
#include <condition_variable>
#include <glm/glm.hpp>
#include <atomic>
#include <vertex.h>
#include <algorithm>

class ChunkQueue
{
public:
  void push(const glm::ivec3 &item)
  {
    std::lock_guard<std::mutex> lock(mtx);
    queue.push_front(item);
    cv.notify_one();
  }

  bool pop(glm::ivec3 &item)
  {
    std::unique_lock<std::mutex> lock(mtx);
    if (queue.empty())
      return false;
    item = std::move(queue.front());
    queue.pop_front();
    return true;
  }

  bool empty() const
  {
    std::lock_guard<std::mutex> lock(mtx);
    return queue.empty();
  }

  bool has(const glm::ivec3 &item) const
  {
    std::lock_guard<std::mutex> lock(mtx);
    return std::find(queue.begin(), queue.end(), item) != queue.end();
  }

private:
  mutable std::mutex mtx;
  std::deque<glm::ivec3> queue;
  std::condition_variable cv;
};

struct CompletedData
{
  glm::ivec3 position;
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

class CompletedQueue
{
public:
  void push(const CompletedData &item)
  {
    std::lock_guard<std::mutex> lock(mtx);
    queue.push(item);
    cv.notify_one();
  }

  bool pop(CompletedData &item)
  {
    std::unique_lock<std::mutex> lock(mtx);
    if (queue.empty())
      return false;
    item = std::move(queue.front());
    queue.pop();
    return true;
  }

  bool empty() const
  {
    std::lock_guard<std::mutex> lock(mtx);
    return queue.empty();
  }

private:
  mutable std::mutex mtx;
  std::queue<CompletedData> queue;
  std::condition_variable cv;
};
