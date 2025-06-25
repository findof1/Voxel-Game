# Voxel-Game

A small real-time 3 D game/engine written in modern C++ and Vulkan.  
It provides the foundations for a voxel or mesh-based world, featuring an ECS-style scene graph, free-fly camera, textured materials, OBJ model loading, and a minimal debug draw pipeline.

---

## ✨ Features
• Vulkan 1.3 renderer using GLFW for window & input.  
• Physically-inspired material system with push-constants.  
• Free-fly first-person camera (WASD + mouse).  
• OBJ + MTL loading via **tinyobjloader**.  
• Image loading through **stb_image**.  
• Text rendering with **FreeType2**.  
• Hot-swappable render commands queue.  
• Simple scene system for creating game objects, adding meshes, and importing external models.

---

## 📁 Repository layout
```text
📂 Include/      → public C++ headers (engine, renderer, components …)
📂 src/          → implementation (.cpp) files
📂 shaders/      → GLSL shaders & `compile.bat` helper script
📂 models/       → sample OBJ assets used by the demo scene
📂 textures/     → PNG textures referenced by the engine & models
CMakePresets.json → build configuration (uses Ninja by default)
```

---

## 🔧 Prerequisites
The engine is developed and tested primarily on **Windows 10** with **MinGW-w64** GCC, but it is fully cross-platform as long as the following dependencies are available:

1. **Vulkan SDK ≥ 1.3** (provides headers, validation layers, `glslc`, …)
2. **GLFW 3.x** (compiled with Vulkan support)
3. **glm** (header-only maths library)
4. **stb_image.h** (header included locally or system-wide)
5. **tinyobjloader** (header-only – included locally or system-wide)
6. **FreeType2**
7. **CMake ≥ 3.21** (for presets) and **Ninja** generator

> On Windows you can obtain most of these through **MSYS2**:
> ```bash
> pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-glfw mingw-w64-x86_64-glm \
>              mingw-w64-x86_64-freetype mingw-w64-x86_64-ninja
> ```

---

## 🛠️ Building
### 1. Clone the repository
```bash
git clone https://github.com/your-username/Voxel-Game.git
cd Voxel-Game
```

### 2. Configure the build directory
The project ships with a ready-made **CMake preset** called `default` that targets MinGW & Ninja on Windows:
```bash
cmake --preset default   # generates build/ and compiles compile_commands.json
```
If you prefer another compiler or OS simply replace the generator:
```bash
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
```

### 3. Build the project
```bash
cmake --build build --config Release
```
The resulting binary will be placed in `build/` (e.g. `build/main.exe`).

---

## 🎨 Compiling the shaders
The two GLSL shaders live in `shaders/`.  A convenience batch script is provided for Windows users:
```powershell
cd shaders
./compile.bat
```
This simply calls `glslc` from the Vulkan SDK and produces `vert.spv` & `frag.spv` which are embedded at runtime.  Linux/macOS users can run the equivalent commands manually:
```bash
glslc shader.vert -o vert.spv
glslc shader.frag -o frag.spv
```
Ensure these SPIR-V files are next to the source GLSL or adjust the paths in `renderer.cpp`.

---

## 🚀 Running the demo
After a successful build just launch the executable:
```bash
./build/main.exe   # or ./build/Voxel-Game on UNIX
```
A window titled "Game Engine" will open and render a simple scene containing:
• A textured skybox.  
• A large ground plane.  
• Two wooden cubes.  
• A scaled OBJ couch model.

---

## 🎮 Default controls
| Action                    | Key / Mouse                 |
|---------------------------|-----------------------------|
| Move forward / backward   | **W** / **S**              |
| Strafe right / left       | **D** / **A**              |
| Ascend / descend          | **Space** / **Left Ctrl**  |
| Speed boost               | **Left Shift**             |
| Look around               | **Hold Right Mouse Button**|
| Zoom                      | **Mouse Wheel**            |
| Toggle cursor lock        | Release Right Mouse Button |

---

## ➕ Extending the engine
Creating a new object from user code (see `src/main.cpp`):
```cpp
Engine engine;
engine.init("My Game", startFn, updateFn);
engine.createGameObject("cube", {0,0,0}, {0,0,0}, {1,1,1});
engine.addMeshToObject("cube", material, "textures/wood.png", cubeVertices, cubeIndices);
```
You can also import any **OBJ/MTL** pair with `Engine::loadModel()` – textures referenced in the MTL file will be loaded automatically.

---

## 📜 License
This project is released under the MIT License – see `LICENSE` for details.  Model and texture assets belong to their respective authors.

---

## 🙏 Acknowledgements
• **Khronos Group** for Vulkan & glslang.  
• **glfw** team for the window/input library.  
• **glm**, **stb_image**, **tinyobjloader**, **Freetype** for awesome single-header libraries. 


TOTALLY NOT GENERATED WITH AI.