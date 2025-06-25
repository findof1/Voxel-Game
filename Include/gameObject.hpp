#include "mesh.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GameObject
{
public:
  glm::vec3 position;
  glm::vec3 rotationZYX;
  glm::vec3 scale = glm::vec3(1.0f);
  std::vector<Mesh> meshes;
};