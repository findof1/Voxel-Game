# Voxel-Game

A small real-time 3 D game/engine written in modern C++ and Vulkan.  
It provides the foundations for a voxel or mesh-based world, featuring an ECS-style scene graph, free-fly camera, textured materials, OBJ model loading, and a minimal debug draw pipeline.

---

## ✨ Features
• Vulkan 1.3 renderer using GLFW for window & input.  
• Free-fly first-person camera and gameplay camera with collision.  
• Voxel world with chunk streaming (16×16×256), biomes, and trees.  
• Multithreaded chunk generation and mesh building.  
• Inventory and hotbar UI with item stacking and counts (FreeType text).  
• Block interaction: raycast to break/place blocks (LMB/RMB).  
• OBJ + MTL loading via **tinyobjloader** (for mesh-based objects).  
• Image loading via **stb_image**; text rendering via **FreeType2**.  
• Skybox, crosshair, hotbar, and item atlas textures.  
• Command-queue based renderer and simple scene system for meshes/UI/text.

---

## 📁 Repository layout
```text
📂 Include/      → public C++ headers (engine, renderer, components …)
📂 Include/VoxelGeneration/ → voxel world, chunks, generators, queues
📄 Include/application.hpp → high-level gameplay loop (voxel demo)
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
A window titled "Game Engine" opens into a procedurally generated voxel world:
• Streaming chunks around the player with a configurable render distance.  
• A textured skybox that follows the player.  
• A visible player capsule (for debugging) and a crosshair.  
• A hotbar displaying collected items with stack counts.

If fonts fail to load for the UI counters, ensure `C:/Windows/Fonts/arial.ttf` exists (or update the path in `Renderer` constructor).

Required textures for the demo UI/atlas (these should be present in `textures/`):
- `sky.png` (skybox)
- `hotbar.png` (hotbar UI)
- `crosshair.png` (crosshair UI)
- `itemAtlas.png` (icons for inventory slots)
- `newAtlas.png` (voxel texture atlas used for world meshes)

---

## 🎮 Default controls
| Action                         | Key / Mouse                  |
|--------------------------------|------------------------------|
| Move forward/backward          | **W** / **S**                |
| Strafe right/left              | **D** / **A**                |
| Jump                           | **Space**                    |
| Sprint                         | **Left Shift**               |
| Look around                    | Move mouse (cursor locked)   |
| Zoom (change FOV)              | **Mouse Wheel**              |
| Break block (targeted)         | **Left Mouse Button**        |
| Place block (selected hotbar)  | **Right Mouse Button**       |
| Select hotbar slot             | **1-9**, **0** (slot 10)     |
| Quit                           | **Esc**                      |

---

## ➕ Extending the engine
Two layers are provided:

- High-level gameplay loop (`Application`) used by the voxel demo:
```cpp
#include <application.hpp>

int main() {
  Application app;
  app.run();
}
```

- Low-level engine API (`Engine`) for manual scene setup/UI/text:
```cpp
Engine engine;
engine.init("My Game", startFn, updateFn);
engine.createGameObject("cube", {0,0,0}, {0,0,0}, {1,1,1});
engine.addMeshToObject("cube", material, "textures/wood.png", cubeVertices, cubeIndices);

// UI and text
engine.createUIObject("crosshair", {engine.WIDTH/2,-500,-7}, {0,0,0}, {20,20,1});
engine.addMeshToObject("crosshair", material, "textures/crosshair.png", squareVertices, squareIndices);
engine.createTextObject("stack0", "64", {10,-10,-6}, {0,0,0}, {0.6f,0.6f,1});

// Import OBJ/MTL meshes
engine.loadModel("couch", "models/couch/couch1.obj", "models/couch/");
```

Voxel world meshing uses a texture atlas; see `Include/VoxelGeneration/` for `World`, `ChunkData`, and `MeshGenerator`.

---

## ⚙️ Tuning & notes
- **Render distance**: change `renderDistance` in `src/application.cpp` (default 4) to trade view distance vs. performance.
- **Chunk size**: `ChunkData::chunkSize = 16`, `ChunkData::chunkHeight = 256` (see `Include/VoxelGeneration/chunkData.hpp`).
- **Multithreading**: `World::startWorker()` is called multiple times to create worker threads for chunk generation.
- **Fonts**: the UI uses FreeType with `arial.ttf`. Adjust the path in `Renderer` if needed.

---

## 📜 License
This project is released under the MIT License – see `LICENSE` for details.  Model and texture assets belong to their respective authors.

---

## 🙏 Acknowledgements
• **Khronos Group** for Vulkan & glslang.  
• **glfw** team for the window/input library.  
• **glm**, **stb_image**, **tinyobjloader**, **Freetype** for awesome single-header libraries.