# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- **Entity-Component-System (ECS):** Integrated `EnTT` for highly performant and decoupled architecture.
- **Event Bus:** Replaced direct input callbacks with `entt::dispatcher` broadcasting `KeyEvent`, `MouseMovedEvent`, and `WindowResizeEvent`.
- **Doxygen Documentation:** Fully automated HTML API documentation generation via GitBook/Doxygen and CMake.

### Changed
- Refactored entire codebase to strict **C++23** standards with RAII and zero raw pointers.
- Upgraded graphics API usage to **OpenGL 4.6 Direct State Access (DSA)**.
- Replaced hardcoded asset arrays with a robust `nlohmann_json` configuration system (`animations.json`).

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
