# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
