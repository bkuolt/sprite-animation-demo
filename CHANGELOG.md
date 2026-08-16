# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.8.0] - 2026-08-16
### Added
- **GPU-Driven Frustum Culling**: Implemented a GLSL 460 compute shader pipeline (`cullComputeShaderSource`) for atomical GPU-side AABB frustum visibility testing.
- **`std430` SSBO Infrastructure**: Added standard SSBO structures (`InstanceData`, `FrustumData`, `DrawElementsIndirectCommand`) with OpenGL 4.6 DSA management.
- **QML Integration Adapter**: Added `GltfQmlItemAdapter` in `src/gltf/GltfQmlItemAdapter.hpp` to enable seamless integration into Qt/QML UI pipelines without adding forced Qt framework build dependencies to core `bgl::gfx`.
- **TextureHandle RAII Migration**: Migrated `TextureCube` to use `TextureHandle` RAII template for memory leak prevention and safety.

## [0.7.2] - 2026-08-16
### Fixed
- **Skybox Cubemap Format**: Fixed a critical texture skewing issue where uncompressed RGB cubemaps were incorrectly uploaded as RGBA to the GPU, causing scrambled colorful noise. The pipeline now dynamically determines the proper `GL_RGB` vs `GL_RGBA` alignment via `ktxTexture_GetElementSize`.
- **Skybox Assets**: Generated and included a proper gradient skybox texture (`skybox.ktx2`) to replace the placeholder colored cubes.

## [0.7.1] - 2026-08-16
### Fixed
- **PBR Color Space Correction**: Fixed physically based rendering math breaking down due to incorrect sRGB to Linear space mapping of BaseColor and Emissive textures.
- **glTF OpenGL Resource Lifecycle**: Resolved a critical bug causing `GL_INVALID_OPERATION` on texture binding due to premature deletion of OpenGL texture handles during the glTF parsing phase.
- **Camera3D Input Handling**: Refactored `Camera3D` to utilize the standard application event bus for scrolling and mouse interactions, restoring 3D zoom and panning capabilities.
- **AMD/Mesa Driver Compatibility**: Mitigated driver crashes by strictly enforcing `GL_RGBA8` internal formats for 2D Array Textures dynamically.
- **Text Rendering Pipeline**: Resolved a critical issue causing HarfBuzz text quads to render as black rectangles by properly disabling global sampler state overrides during SSBO texture generation.
- **Camera Orientation**: Fixed `Camera3D` pitch calculations and configured the default scene camera to an isometric top-down perspective.

### Added
- **OpenGL Architecture Isolation**: Migrated all core OpenGL objects to a dedicated `bgl::gl` namespace and established strict component boundaries within the CMake build system.
- **Skybox & Cubemap System**: Implemented an RAII-compliant `TextureCube` class for KTX2 cubemap support and integrated a fully functional 3D skybox rendering pass with optimized inverse-projection math.

### Changed
- Scaled up the default glTF model instantiation at startup via the Scene root transform matrix.

## [0.7.0] - 2026-08-16
### Added
- **glTF Asset Pipeline**: Integrated full support for loading 3D glTF models with embedded textures via `fastgltf` and `libjpeg`.
- **PBR Render Engine**: Implemented a complete Cook-Torrance BRDF pipeline for physically based rendering.
- **Lua Scripting**: Created a new isolated static library for Lua scripting utilizing `sol3` via Conan.

## [0.6.0] - 2026-08-02
### Added
- **OpenAL Audio Subsystem**: Integrated a procedural and RAII-compliant `AudioEngine` using OpenAL Soft.

## [0.5.0] - 2026-07-26
### Added
- **Core Decoupling & Event Bus:** Integrated the `EnTT` framework to establish a highly performant, type-safe Event Bus architecture (`entt::dispatcher`).
- **Asynchronous Event Handling:** Input events (`KeyEvent`, `MouseMovedEvent`, `MouseButtonEvent`, `ScrollEvent`, `WindowResizeEvent`) are now broadcast globally, entirely decoupling the `Window` and `InputHandler` from the core `Application` game logic.
- **Doxygen & GitBook Documentation:** Fully automated HTML API documentation generation via CMake `FetchContent` (using the modern `doxygen-awesome-css` theme). The `docs/` directory is now fully structured for GitBook deployments (`SUMMARY.md`, `architecture.md`, `api.md`).
- **Agent Memory Guidelines:** Created `AGENT_MEMORY.md` to persistently store architectural constraints (C++23, OpenGL DSA, EnTT ECS) for future AI agent interactions.
- **High-Quality Placeholder Imagery:** Automatically generated and integrated a professional 16-bit pixel art placeholder screenshot into `README.md`.

### Changed
- **C++23 Modernization:** Complete refactoring of the entire codebase to strict C++23 standards, eliminating all raw pointers and enforcing rigorous RAII memory safety paradigms via `std::unique_ptr` and `std::shared_ptr`.
- **OpenGL 4.6 DSA Migration:** Upgraded the graphics pipeline to exclusively utilize OpenGL 4.6 Direct State Access (DSA) for texture, sampler, and buffer object management (`glCreateTextures`, `glCreateSamplers`), significantly reducing state mutation errors.
- **JSON Asset Pipeline:** Replaced hardcoded C++ asset arrays with a robust runtime configuration system powered by `nlohmann_json`, loading sprite animation definitions directly from `assets/animations.json`.
- **Global Copyright Header:** Standardized copyright attribution headers across all 38 C++ source and header files globally.
- **Environment Setup Script:** Enhanced `install.sh` with professional, idiomatic English phrasing, improved layout, and clearer feedback for setting up system packages and Conan dependencies.

## [0.4.0] - 2026-07-26
### Added
- Implemented SPIR-V pre-compiled shader loading.
- Added dynamic Orthographic 2D Camera with mouse panning and zooming.

## [0.3.0] - 2026-07-25
### Changed
- Migrated dependency management to Conan 2.x and `conan.lock` for reproducible builds.
- Refactored renderer to support KTX2/Basis Universal texture compression.

## [0.2.3] - 2026-07-20
### Fixed
- Addressed memory leaks in shader compilation and vertex buffer allocation.

## [0.2.2] - 2026-07-15
### Added
- Introduced initial fallback loaders for `libpng` and `libjpeg`.

## [0.2.1] - 2026-07-10
### Changed
- Minor build system improvements (CMake refactoring).

## [0.2.0] - 2026-07-05
### Added
- HarfBuzz text shaping integration for HUD and UI overlays.

## [0.1.0] - 2026-07-01
### Added
- Initial prototype release of the Sprite Animation Engine.
- Basic OpenGL fixed-function fallback and legacy rendering support.
