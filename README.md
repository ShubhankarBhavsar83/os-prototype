# os-prototype-v2 Engine Architecture & Technical Specification

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![SDL3](https://img.shields.io/badge/SDL-3.2.20-orange.svg)](https://wiki.libsdl.org/SDL3)
[![Build](https://img.shields.io/badge/Build-CMake-brightgreen.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

`os-prototype-v2` is a high-performance **2D Isometric Action-RPG Engine** written in C++20. It features a custom **7-layer z-sorted depth renderer**, spatial collision matrix, hardware-accelerated integer scaling, and a **multithreaded asynchronous API client** linking in-game UI to dynamic Large Language Model (LLM) backends.

---

## Engine Architecture & Component Topology

```text
                         +-----------------------------------+
                         |            Application            |
                         |   (Main Loop, SDL3 Window/Render) |
                         +-----------------+-----------------+
                                           |
                                           v
                         +-----------------+-----------------+
                         |            GameState              |
                         |  (Layer Management, Camera, Modes)|
                         +----+------------+------------+----+
                              |            |            |
         +--------------------+            |            +--------------------+
         v                                 v                                 v
+------------------+             +-------------------+             +-------------------+
|  7-Layer Matrix  |             |  Collision Engine |             | ResourceManager   |
| (Depth Sorting)  |             | (Circle/AABB/Dist)|             | (Texture/Font Cache)|
+------------------+             +-------------------+             +-------------------+
         |                                 |
         +-----------------+---------------+
                           |
                           v
            +--------------+--------------+
            |        Entity System        |
            | (Player, NPCs, Projectiles) |
            +--------------+--------------+
                           |
                           v (On Interaction)
            +--------------+--------------+
            |         ChatSystem          |
            | (SDL3_ttf Text UI Engine)   |
            +--------------+--------------+
                           |
                           v (Async Thread Dispatch)
            +--------------+--------------+
            |          APIClient          |
            | (libCURL Thread Worker Queue)|
            +-----------------------------+
```

---

## Systems Overview & Deep Technical Breakdown

### 1. 7-Layer Depth Rendering Matrix (`GameState`)
To resolve 2D isometric depth overlap without expensive per-frame sorting overhead, `GameState` maintains a fixed 7-layer array of entity vectors (`std::array<std::vector<std::unique_ptr<Entity>>, 7>`):

| Layer Index | Constant Identifier | Layer Purpose & Occlusion Hierarchy |
| :--- | :--- | :--- |
| **0** | `LAYER_IDX_LEVEL` | Base isometric floor grid & terrain tiles. |
| **1** | `LAYER_IDX_FURNITURE_BACKGROUND` | Rear environmental objects, floor mats, low furniture. |
| **2** | `LAYER_IDX_PORTAL_BACKGROUND` | Back visual elements of level entry/exit portals. |
| **3** | `LAYER_IDX_CHARACTERS` | Dynamic entities: Player, Friendly NPCs, Enemy NPCs. |
| **4** | `LAYER_IDX_PROJECTILES` | Spell projectiles, ranged attacks, active hitboxes. |
| **5** | `LAYER_IDX_PORTAL_FOREGROUND` | Front visual portal overlay frame (occludes characters). |
| **6** | `LAYER_IDX_FURNITURE_FOREGROUND` | High scenery elements, tree crowns, roofs (occludes characters). |

---

### 2. Asynchronous Multithreaded Networking Engine (`APIClient`)
The engine implements a non-blocking worker thread loop to handle HTTP POST payloads to LLM endpoints without stalling the main game thread render pipeline:

- **Thread Concurrency**: Worker thread (`std::thread workerThread`) executing `APIClient::workerLoop()`.
- **Synchronization**: Two distinct thread queues (`requestQueue`, `responseQueue`) protected by `std::mutex` and `std::lock_guard`.
- **HTTP Transport**: `libCURL` easy interface configured with callback delegates (`WriteCallback`) handling dynamic payloads (`Content-Type: application/json`).
- **Data Flow**:
  1. `FriendlyNPC` triggers prompt input via `ChatSystem`.
  2. Request payload is pushed to `APIClient::requestQueue`.
  3. Worker thread dequeues request, executes `makeHTTPRequest()`, and invokes callback delegate upon payload receipt.
  4. Response text is safely extracted in `FriendlyNPC::update()` and appended to `ChatSystem`.

---

### 3. Coordinate System & Camera Viewport
- **Isometric Math**: Transforms world grid coordinates to screen pixel offsets using projection matrices.
- **Hardware Integer Scaling**: Built on SDL3 logical presentation (`SDL_LOGICAL_PRESENTATION_INTEGER_SCALE`) maintaining clean pixel art ratio:
  - Native Screen Window: `1280 x 720` (or Fullscreen mode).
  - Internal Logical Viewport: `640 x 320`.
- **VSync Synchronization**: `SDL_SetRenderVSync(renderer, 1)` for tearing-free rendering.

---

### 4. Entity & Physics Engine (`Entity`, `Movable`, `Collider`)
- **Physics Integrator**: Delta-time vector integration with customizable friction decay (`friction = 900.0f`), max speed caps (`maxSpeedX`, `maxSpeedY`), and velocity updates.
- **Collision Matrices**:
  - **AABB Box Collisions**: Environmental boundaries and furniture bounds.
  - **Radial Distance Checks**: Range queries for NPC dialogue activation (`findNearestFriendlyNPC`), melee attack hitboxing, and projectile impacts.
- **AI State Machines**: Enemy NPCs compute distance-based pathing and damage timers targeting the player.

---

## Build Matrix & Dependency Specification

### Prerequisites
| Tool / Library | Required Version | Purpose |
| :--- | :--- | :--- |
| **C++ Compiler** | C++20 compliant (MSVC 2022 / GCC 11+ / Clang 13+) | Language standard & modern features |
| **CMake** | `>= 3.12` | Cross-platform build generator |
| **SDL3** | `>= 3.2.20` | Core windowing, input, hardware renderer |
| **SDL3_image** | `>= 3.2.4` | Texture loading pipeline |
| **SDL3_ttf** | `>= 3.2.2` | Font rendering engine |
| **GLM** | `>= 0.9.9` | Vector and matrix mathematics |
| **libCURL** | `>= 8.5.0` (Auto-fetched) | Async HTTP client networking |

---

### Building from Source (Windows / MSVC / Ninja)

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/ShubhankarBhavsar83/os-prototype.git
   cd os-prototype
   ```

2. **Configure CMake Prefix Paths**:
   Ensure `CMAKE_PREFIX_PATH` in `os-prototype-v2/CMakeLists.txt` points to your local SDL3 and GLM library installations:
   ```cmake
   list(APPEND CMAKE_PREFIX_PATH "C:/path/to/SDL3-3.2.20")
   list(APPEND CMAKE_PREFIX_PATH "C:/path/to/SDL3_image-3.2.4")
   list(APPEND CMAKE_PREFIX_PATH "C:/path/to/SDL3_ttf-3.2.2")
   ```

3. **Generate Build System & Compile**:
   ```bash
   # Create build directory and configure
   cmake -B out/build -S .

   # Build executable
   cmake --build out/build --config Release
   ```
   *Note: A CMake post-build custom command automatically copies `libcurl.dll` to the output executable binary folder.*

---

## 📂 Repository Directory Tree

```text
os-prototype-v2/
├── CMakeLists.txt              # Top-level CMake configuration
├── assets/                     # Game assets (sprites, maps, fonts)
│   ├── enemy_assets/
│   ├── font/
│   ├── levels/
│   ├── map_assets/
│   ├── npcs/
│   └── player_assets/
└── os-prototype-v2/            # Core Engine Source Code
    ├── CMakeLists.txt          # Target dependencies & link flags
    ├── application.h/cpp       # Windowing, SDL3 init, main loop
    ├── gameState.h/cpp         # 7-layer matrix, spatial queries
    ├── apiClient.h/cpp         # Async HTTP worker thread (libCURL)
    ├── chatSystem.h/cpp        # TTF text rendering & chat box UI
    ├── levelLoader.h/cpp       # Level map loader & entity spawner
    ├── uiManager.h/cpp         # UI layout manager
    ├── player.h/cpp            # Player entity, movement, combat
    ├── enemyNpc.h/cpp          # Hostile NPC state machine & AI
    ├── friendlyNpc.h/cpp       # Interactive NPC & chat trigger
    ├── entity.h                # Base entity class
    ├── movable.h               # Physics & velocity traits
    ├── collider.h              # Collider boundaries
    ├── coordinateSystem.h      # Isometric transform utilities
    ├── resourceManager.h       # Texture & font cache system
    └── os-prototype-v2.cpp     # Entry point (`main`)
```

---

## Controls & Inputs

| Input | Action | Mode |
| :--- | :--- | :--- |
| `W` `A` `S` `D` / Arrow Keys | Move Player Entity | Exploration / Combat |
| `Space` / `Left Click` | Melee Attack / Cast Projectile | Exploration / Combat |
| `E` | Initiate NPC Interaction | Near Friendly NPC |
| `Enter` | Send Chat Message | Chatting Mode |
| `Esc` | Pause / Unpause Game State | Any |

---

## License & Acknowledgments

This project is open-source under the **MIT License**.
- Built with [SDL3](https://www.libsdl.org/) and [GLM](https://github.com/g-truc/glm).
- Networking powered by [cURL](https://curl.se/).
- Companion Level Editor repository: [level-editor-SDL3-osiris](https://github.com/ShubhankarBhavsar83/level-editor-SDL3-osiris) (forked from team project repository).
