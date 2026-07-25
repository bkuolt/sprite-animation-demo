# OpenGL Sprite Animation Demo

![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![OpenGL 4.6](https://img.shields.io/badge/OpenGL-4.6-blue.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

A high-performance 2D sprite animation demo built with modern C++23 and OpenGL 4.6. This project showcases polymorphic texture loading (`KTX2`, `PNG`, `JPEG`), callback-driven windowing, GPU frame-tweening, and SPIR-V shaders.

---

## ✨ Features

- **Modern Graphics Pipeline**: Utilizes Direct State Access (DSA) in OpenGL 4.6 for a clean, state-decoupled, and efficient rendering pipeline.
- **Polymorphic Texture Loaders**: Abstracted texture loading interface (`bgl::ITextureLoader`) supporting `KTX2` (Basis Universal compressed), `PNG` (`libpng`), and `JPEG` (`libjpeg`) textures uploaded to `GL_TEXTURE_2D_ARRAY`.
- **Callback-Driven Architecture**: Decoupled rendering and event handling where application logic resides in `main.cpp` and window management is delegated to `Window`.
- **SPIR-V Shaders**: Pre-compiled SPIR-V binary shaders loaded directly via OpenGL 4.6 `glShaderBinary` and specialized with `glSpecializeShaderARB`.
- **GPU-based Animation & Interpolation**: Smooth frame-to-frame tweening performed directly on the GPU within `sampler2DArray` textures.
- **Modern C++23 Standard**: Written in strict C++23 using modern language features (`std::span`, `std::println`, smart pointers, `#pragma once`, zero macro pollution).
- **Graceful Shutdown**: Intercepts `SIGINT` / `SIGTERM` signals for clean resource release and window termination.
- **Build System**: Clean build setup with CMake 3.20+, Conan 2.x, and Taskfile automation.

---

## 🛠️ Tech Stack

- **Language**: C++23
- **Graphics API**: OpenGL 4.6 (Core Profile, DSA, SPIR-V)
- **Windowing & Input**: GLFW 3.4
- **Dependencies**:
  - `glad`: OpenGL 4.6 Loader
  - `glm`: OpenGL Mathematics
  - `ktx`: Khronos KTX2 & Basis Universal Transcoder
  - `libpng`: Portable Network Graphics library
  - `libjpeg`: Independent JPEG Group's JPEG library
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

### Build & Run

Using **Taskfile**:

```bash
# 1. Install dependencies via Conan
task setup

# 2. Compile shaders and build C++ executable
task build

# 3. Launch application
task run
```

Or manually:

```bash
./compile-shaders.sh
cmake --preset conan-release
cmake --build --preset conan-release
./build/Release/app
```

---

## 🎮 Controls

- **`Spacebar`**: Cycle through animation states (Idle, Walk, Jump, Run, Slide, Dead).
- **`ESC` / `Ctrl+C`**: Gracefully exit application.

---

## 📜 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.