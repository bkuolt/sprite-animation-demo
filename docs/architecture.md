# Architecture

This project is built around the following key technologies:
- **C++23**: Strict RAII, std::move semantics, and forward-looking language features.
- **OpenGL 4.6 Core**: Exclusively utilizing Direct State Access (DSA) for managing buffers, textures, and samplers.
- **JSON Assets**: Using `nlohmann/json` to load character mappings dynamically at runtime.
- **Dependency Management**: Conan 2 is used to manage libraries like `glad`, `glfw`, `spdlog`, `fmt`, etc.
