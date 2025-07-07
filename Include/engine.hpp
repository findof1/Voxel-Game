#include "renderer.hpp"
#include "gameObject.hpp"
#include "debugDrawer.hpp"
struct Input
{
  bool keys[GLFW_KEY_LAST] = {false};
  bool mouseButtons[GLFW_MOUSE_BUTTON_LAST] = {false};
  double mouseX = 0.0;
  double mouseY = 0.0;
  double scrollOffsetX = 0.0;
  double scrollOffsetY = 0.0;
};

class Engine
{
public:
  Camera camera;
  uint32_t WIDTH;
  uint32_t HEIGHT;
  Input input;

  std::unordered_map<std::string, GameObject> gameObjects;
  std::unordered_map<std::string, GameObject> uiObjects;
  VulkanDebugDrawer *linesDrawer = nullptr;

  Engine(uint32_t width = 1600, uint32_t height = 1000) : WIDTH(width), HEIGHT(height), camera(), renderer(camera, WIDTH, HEIGHT)
  {
  }

  void init(std::string windowName, std::function<void(Engine *)> startFn, std::function<void(Engine *, float)> updateFn);
  void run();
  void shutdown();

  void render();

  void clearHierarchy();

  inline void moveCamera(const glm::vec3 &offset)
  {
    camera.move(offset);
  }

  inline void moveCameraForwards(float amount)
  {
    camera.moveForwards(amount);
  }

  inline void moveCameraRight(float amount)
  {
    camera.moveRight(amount);
  }

  inline void rotateCamera(float yawOffset, float pitchOffset, bool constrainPitch = true)
  {
    camera.rotate(yawOffset, pitchOffset, constrainPitch);
  }

  inline void setCameraZoom(float zoom)
  {
    camera.setZoom(zoom);
  }

  void useFreeCamMode(bool activate)
  {
    autoFreeCam = true;
  }

  std::shared_ptr<TextureManager> voxelTextureAtlas;
  void addVoxelMeshToObject(std::string identifier, MaterialData material, const std::string &texturePath, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);

  void createGameObject(std::string identifier, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
  void removeGameObject(std::string identifier);
  void addMeshToObject(std::string identifier, MaterialData material, const std::string &texturePath, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
  void loadModel(std::string identifier, const std::string objPath, const std::string mtlPath);

  void createUIObject(std::string identifier, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

  void disableCursor();
  void enableCursor();

private:
  Renderer renderer;
  GLFWwindow *window;
  int nextRenderingId = 0;
  std::function<void(Engine *, float)> update;
  std::function<void(Engine *)> start;
  bool autoFreeCam = false;

  void initWindow(std::string windowName);

  static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    app->renderer.framebufferResized = true;
  }

  static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    app->input.mouseX = xpos;
    app->input.mouseY = ypos;
  }

  static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    app->input.scrollOffsetX = xoffset;
    app->input.scrollOffsetY = yoffset;
  }

  static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    if (key >= 0 && key < GLFW_KEY_LAST)
    {
      app->input.keys[key] = (action != GLFW_RELEASE);
    }
  }

  static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    if (button >= 0 && button < GLFW_MOUSE_BUTTON_LAST)
    {
      app->input.mouseButtons[button] = (action != GLFW_RELEASE);
    }
  }

  void updateFreeCam(float dt);

  bool isRunning = true;
};