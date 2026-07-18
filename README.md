# OpenGL Sprite Animation Demo

![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![OpenGL 4.6](https://img.shields.io/badge/OpenGL-4.6-blue.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

A high-performance 2D sprite animation demo built with modern C++23 and OpenGL 4.6. This project showcases efficient rendering techniques, including texture arrays for animations, SPIR-V shaders, and KTX2 textures with Basis Universal compression.


## ✨ Features

- **Modern Graphics Pipeline**: Utilizes modern OpenGL 4.6 features, primarily Direct State Access (DSA), for a cleaner and more efficient rendering pipeline.
- **SPIR-V Shaders**: GLSL shaders are pre-compiled to the SPIR-V binary format for faster shader loading and reduced driver overhead at runtime.
- **Efficient Animation**: Animations are stored in `sampler2DArray` texture arrays. Frame-to-frame tweening (interpolation) is performed directly on the GPU for perfectly smooth animations, regardless of frame rate.
- **Advanced Texture Compression**: Uses the KTX2 container format with Basis Universal (transcoded to BC3/DXT5) for significantly reduced VRAM usage and faster loading times.
- **Modern C++23**: Written in C++23, leveraging modern language features for cleaner, safer, and more expressive code.
- **Robust Build System**: Uses CMake for project configuration, Conan for dependency management, and Taskfile for streamlined build and run commands.

## 🛠️ Tech Stack

- **Language**: C++23
- **Graphics API**: OpenGL 4.6
- **Windowing**: GLFW
- **Dependencies**:
    - `glad`: OpenGL Loading Library
    - `glm`: OpenGL Mathematics
    - `spdlog`: Fast C++ logging library
    - `fmt`: Formatting library
    - `ktx`: Khronos Texture library for KTX2 loading
- **Build & Automation**:
    - `CMake`: Build System
    - `Conan`: Dependency Manager
    - `Taskfile`: Task Runner
    - `glslc` / `glslangValidator`: For SPIR-V compilation

## 🚀 Getting Started

Follow these steps to build and run the project on a Linux-based system.

### Prerequisites

- A C++23 compatible compiler (e.g., GCC 12+, Clang 15+).
- `CMake` (version 3.15+).
- `Conan 2.x`.
- `Python 3` (for the shader compilation script).
- `glslc` (from the Vulkan SDK).
- OpenGL 4.6 compatible drivers.

### Build & Run Steps

This project uses `Taskfile` to simplify the process. Make sure you have Task installed.

1.  **Clone the repository:**
    ```bash
    git clone <your-repo-url>
    cd sprite-animation-demo
    ```

2.  **Install Dependencies:**
    This command uses Conan to fetch and install all required libraries.
    ```bash
    task setup
    ```

3.  **Build the Project:**
    This command first compiles the shaders and then builds the C++ application.
    ```bash
    task build
    ```

4.  **Run the Application:**
    ```bash
    task run
    ```

Alternatively, you can run the individual steps manually:
- `task shaders`: Compile shaders.
- `cmake --preset conan-release`: Configure CMake.
- `cmake --build --preset conan-release`: Build the project.
- `./build/Release/app`: Run the executable.

## 🎮 Controls

- **`Spacebar`**: Switch to the next animation (e.g., from Idle to Walk).
- **`ESC`**: Close the application.

## 💡 How It Works

The core of the animation system relies on two key OpenGL features:

1.  **Texture Arrays (`sampler2DArray`)**: Each animation (idle, walk, run, etc.) is loaded from a `.ktx2` file into its own texture array. Each layer of the array represents a single frame of the animation. This is highly efficient as all frames for an animation are stored in a single texture object, minimizing state changes and texture binds.

2.  **GPU-side Interpolation**: The vertex shader receives the current time. It calculates the current frame index and a `tweenFactor` (a float from 0.0 to 1.0 representing the progress between two frames). These values are passed to the fragment shader. The fragment shader then samples *both* the current frame and the next frame from the texture array and uses `mix()` to linearly interpolate between them based on the `tweenFactor`. This results in ultra-smooth animation that is decoupled from the application's frame rate.

## 📜 License

This project is licensed under the MIT License. See the `LICENSE` file for details.