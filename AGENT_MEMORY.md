# Agent Memory & Architectural Guidelines

This document serves as persistent memory and strict guidelines for AI agents working on this repository.

## 1. Core Technologies & Standards
- **C++ Standard:** C++23 ONLY. Do not use legacy features, `new`/`delete`, or raw owning pointers. Use `std::unique_ptr`, `std::shared_ptr`, and `std::span` universally.
- **Graphics API:** OpenGL 4.6 (Core Profile) exclusively.
- **State Management:** **Direct State Access (DSA)** must be used for all OpenGL objects (`glCreate*`, `glNamed*`, `glTextureParameteri`). Never use `glBind*` for state configuration, only for rendering.
- **Dependency Management:** Handled by Conan 2.x. If a new dependency is needed, add it to `conanfile.txt` and search `conancenter`.
- **Build System:** CMake 3.20+ and Taskfile (`task build`, `task run`).

## 2. Architecture Details
- **Entity-Component-System (ECS):** The engine uses **EnTT** (`entt::registry`) for entity management. Do not use standard Object-Oriented inheritance trees for game entities (e.g., `Character`, `Monster`). Use components.
- **Event Bus:** Input and Window events are dispatched using `entt::dispatcher`. The `Application` subscribes to these events. Avoid hardcoded `std::function` callbacks between systems.
- **Memory Safety:** Enforce strict RAII. All OpenGL handles (`GLuint`) must be wrapped in classes that delete them in their destructor (e.g., `glDeleteTextures`, `glDeleteVertexArrays`). Delete copy constructors on resource wrappers to prevent double-free errors.
- **Shaders:** Use SPIR-V for production shaders (`.spv`). Fallbacks can use GLSL loaded via `BGL_ENABLE_GLSL_LOADER`.

## 3. Formatting & Style
- Run `clang-format` after making significant code edits.
- Use `m_` prefixes for private class members.
- Use `camelCase` for methods and variables.
- Use `PascalCase` for classes and structs.
- Copyright header on all files: `// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.`

## 4. Documentation
- Use Doxygen-style docstrings (`///` or `/** */`) for public APIs.
- Keep the GitBook architecture updated (`docs/SUMMARY.md`).
