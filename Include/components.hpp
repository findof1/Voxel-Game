#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "mesh.hpp"

struct TransformComponent
{
  glm::vec3 position;
  glm::vec3 rotationZYX;
  glm::vec3 scale = glm::vec3(1.0f);
  bool justUpdated = true;
};

struct MeshComponent
{
  bool hide = false;
  std::vector<Mesh> meshes;

  bool loadedFromFile = false;
  std::string objPath;
  std::string mtlPath;
};

struct BoxColliderComponent
{
  bool justUpdated = true;

  glm::vec3 localMin = glm::vec3(-0.5f);
  glm::vec3 localMax = glm::vec3(0.5f);

  glm::vec3 worldMin;
  glm::vec3 worldMax;

  bool autoUpdate = false;

  // serialization stuff
  glm::vec3 position;
  glm::vec3 rotationZYX;
  glm::vec3 scale;

  void updateWorldAABB(const glm::vec3 &positionIn, const glm::vec3 &rotationZYXIn, const glm::vec3 &scaleIn)
  {
    position = positionIn;
    rotationZYX = rotationZYXIn;
    scale = scaleIn;

    justUpdated = true;
    glm::quat rotation = glm::quat(glm::radians(rotationZYX));

    glm::vec3 scaledMin = localMin * scale;
    glm::vec3 scaledMax = localMax * scale;

    glm::vec3 corners[8] = {
        glm::vec3(scaledMin.x, scaledMin.y, scaledMin.z),
        glm::vec3(scaledMax.x, scaledMin.y, scaledMin.z),
        glm::vec3(scaledMin.x, scaledMax.y, scaledMin.z),
        glm::vec3(scaledMax.x, scaledMax.y, scaledMin.z),
        glm::vec3(scaledMin.x, scaledMin.y, scaledMax.z),
        glm::vec3(scaledMax.x, scaledMin.y, scaledMax.z),
        glm::vec3(scaledMin.x, scaledMax.y, scaledMax.z),
        glm::vec3(scaledMax.x, scaledMax.y, scaledMax.z)};

    worldMin = glm::vec3(FLT_MAX);
    worldMax = glm::vec3(-FLT_MAX);

    for (const auto &corner : corners)
    {
      glm::vec3 rotatedCorner = rotation * corner;
      glm::vec3 worldCorner = rotatedCorner + position;

      worldMin = glm::min(worldMin, worldCorner);
      worldMax = glm::max(worldMax, worldCorner);
    }
  }

  std::vector<glm::vec3> getWorldCorners(const glm::vec3 &position, const glm::vec3 &rotationZYX, const glm::vec3 &scale) const
  {
    glm::quat rotation = glm::quat(glm::radians(rotationZYX));
    glm::vec3 scaledMin = localMin * scale;
    glm::vec3 scaledMax = localMax * scale;

    glm::vec3 corners[8] = {
        glm::vec3(scaledMin.x, scaledMin.y, scaledMin.z),
        glm::vec3(scaledMax.x, scaledMin.y, scaledMin.z),
        glm::vec3(scaledMin.x, scaledMax.y, scaledMin.z),
        glm::vec3(scaledMax.x, scaledMax.y, scaledMin.z),
        glm::vec3(scaledMin.x, scaledMin.y, scaledMax.z),
        glm::vec3(scaledMax.x, scaledMin.y, scaledMax.z),
        glm::vec3(scaledMin.x, scaledMax.y, scaledMax.z),
        glm::vec3(scaledMax.x, scaledMax.y, scaledMax.z),
    };

    std::vector<glm::vec3> worldCorners;
    for (const glm::vec3 &corner : corners)
    {
      glm::vec3 rotatedCorner = rotation * corner;
      worldCorners.push_back(rotatedCorner + position);
    }
    return worldCorners;
  }

  void getWorldAxes(const glm::vec3 &rotationZYX, glm::vec3 axes[3]) const
  {
    glm::quat rotation = glm::quat(glm::radians(rotationZYX));
    axes[0] = rotation * glm::vec3(1, 0, 0);
    axes[1] = rotation * glm::vec3(0, 1, 0);
    axes[2] = rotation * glm::vec3(0, 0, 1);
  }

  bool containsPoint(const glm::vec3 &point) const
  {
    return (point.x >= worldMin.x && point.x <= worldMax.x) &&
           (point.y >= worldMin.y && point.y <= worldMax.y) &&
           (point.z >= worldMin.z && point.z <= worldMax.z);
  }
};

struct RigidBodyComponent
{
  glm::vec3 velocity = glm::vec3(0.0f);

  glm::vec3 acceleration = glm::vec3(0.0f);

  float mass = 1.0f;

  bool useGravity = true;

  bool isStatic = false;

  void applyForce(const glm::vec3 &force)
  {
    if (isStatic || mass <= 0.0f)
      return;
    acceleration += force / mass;
  }

  void integrate(float deltaTime)
  {
    if (isStatic)
      return;

    if (useGravity)
    {
      glm::vec3 gravity(0.0f, -9.81f, 0.0f);
      acceleration += gravity;
    }

    velocity += acceleration * deltaTime;

    acceleration = glm::vec3(0.0f);
  }

  void applyVelocity(TransformComponent &transform, float deltaTime)
  {
    if (isStatic)
      return;
    transform.position += velocity * deltaTime;
    transform.justUpdated = true;
  }
};