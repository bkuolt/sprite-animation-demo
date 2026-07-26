# OpenGL Sprite Animation Demo

![Screenshot](assets/screenshot.png)

![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![OpenGL 4.6](https://img.shields.io/badge/OpenGL-4.6-blue.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

Modernized C++23 OpenGL sprite animation demonstration featuring Direct State Access (DSA), `nlohmann_json` asset configuration, HarfBuzz text shaping, and automated Doxygen (GitBook) documentation.

---

## ✨ Features

- **Modern Graphics Pipeline**: Utilizes Direct State Access (DSA) in OpenGL 4.6 for a clean, state-decoupled, and efficient rendering pipeline.
- **Interactive 2D Camera**: Orthographic camera (`glm::ortho`) with mouse-drag panning, mouse-wheel zoom, camera position reset (`R` key), and aspect-ratio preservation.
- **Polymorphic Texture Loaders**: Abstracted texture loading interface (`bgl::ITextureLoader`) supporting `KTX2` (Basis Universal compressed), `PNG` (`libpng`), and `JPEG` (`libjpeg`) textures uploaded to `GL_TEXTURE_2D_ARRAY`.
- **Callback-Driven Architecture**: Decoupled rendering and event handling where application logic resides in `main.cpp` and window management is delegated to `Window`.
- **SPIR-V Shaders**: Pre-compiled SPIR-V binary shaders loaded directly via OpenGL 4.6 `glShaderBinary` and specialized with `glSpecializeShaderARB`.
- **GPU-based Animation & Interpolation**: Smooth frame-to-frame tweening performed directly on the GPU within `sampler2DArray` textures.
- **Modern C++23 Standard**: Written in strict C++23 using modern language features (`std::span`, `std::println`, smart pointers, `#pragma once`, zero macro pollution).
- **CMake Options**: Modular build with optional `PNG`, `JPEG`, and `GLSL` source loader support (`BGL_ENABLE_PNG_LOADER`, `BGL_ENABLE_JPEG_LOADER`, `BGL_ENABLE_GLSL_LOADER`).
- **Asset Pipeline**: Includes `tools/convert_to_ktx2.py` for automated PNG to KTX2 conversions using Basis Universal and Zstandard.
- **Graceful Shutdown**: Intercepts `SIGINT` / `SIGTERM` signals for clean resource release and window termination.
- **Build System**: Clean build setup with CMake 3.20+, Conan 2.x (with `conan.lock`), and Taskfile automation.

---

## 🛠️ Tech Stack

- **Language**: C++23
- **Graphics API**: OpenGL 4.6 (Core Profile, DSA, SPIR-V)
- **Windowing & Input**: GLFW 3.4
- **Dependencies**:
  - `glad`: OpenGL 4.6 Loader
  - `glm`: OpenGL Mathematics
  - `ktx`: Khronos KTX2 & Basis Universal Transcoder
  - `libpng` & `libjpeg`: PNG and JPEG fallback loaders
  - `fontconfig` & `freetype`: System font resolution and text shaping
  - `harfbuzz`: Advanced text shaping for UI overlays
  - `spdlog` & `fmt`: Fast logging and string formatting
- **Build Automation**:
  - `CMake`: Build System
  - `Conan`: Package Manager
  - `Taskfile`: Command Task Runner

---

## 🚀 Getting Started

### Prerequisites

- C++23 compatible compiler (GCC 13+, Clang 16+, or MSVC 2022+).
- `CMake` (3.20+).
- `Conan 2.x`.
- `Taskfile` (`task`).

### Automated Setup

Run the automated setup script after cloning:

```bash
./install.sh
```

Or using **Taskfile**:

```bash
# 1. Install dependencies via Conan (using conan.lock)
task setup

# 2. Compile shaders and build C++ executable
task build

# 3. Launch application
task run
```

---

## 🎮 Controls

- **`Maus drag (Gedrückte Maustaste + Ziehen)`**: Kamera pannen (Verschieben der 2D-Kameraansicht).
- **`Mausrad Hoch / Runter`**: Rein- und Rauszoomen (Kamera-Zoom).
- **`Taste R`**: Kamera zurücksetzen (Position `(0, 0)` & Zoom `1.0`).
- **`Pfeil-oben` / `Leertaste`**: Schaltet zur nächsten Animation des aktuellen Charakters.
- **`Pfeil-unten`**: Schaltet zur vorherigen Animation.
- **`Pfeil-links` / `Pfeil-rechts`**: Wechselt zwischen den geladenen Charakteren (z.B. "Hero" und "Villain").
- **`ESC` / `Ctrl+C`**: Anwendung sauber beenden.

---

## 📜 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.