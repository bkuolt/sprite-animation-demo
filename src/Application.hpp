// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

// Forward declarations for heavy subsystems — prevents their full headers from
// being compiled into every translation unit that includes Application.hpp.
// The concrete types are only needed in Application.cpp.
#include "events/Event.hpp"   // Lightweight event structs (no GL, no audio)
#include "gfx/QuadMesh.hpp"   // QuadMesh is used as a value member — cannot forward-declare
#include <entt/entt.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glad/gl.h>
#include "gl/GLHandle.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

// Forward declarations — heavy enough that every translation unit does NOT need
// their full definition to compile.
namespace bgl::window  { class Window; }
namespace bgl::gfx     { class Camera; class Hud; class TileMap; class Texture2DArray; class Sampler; class Scene; class GltfRenderer; class Grid; class Camera3D; }
namespace bgl::audio   { class AudioEngine; }

namespace bgl
{
class Character;
class Font;  // Defined in gfx/text/Font.hpp (namespace bgl, pulls in FreeType/HarfBuzz)

/**
 * @brief Main engine application class.
 *
 * Manages the render loop, windowing, asset loading, input, audio, and
 * entity rendering. All heavy subsystems are stored via unique_ptr to
 * allow forward-declaration in this header.
 */
class Application
{
  public:
    /**
     * @brief Initializes all subsystems, window, events, and default graphics state.
     */
    Application();

    /**
     * @brief Releases all OpenGL resources, programs, and buffers.
     */
    ~Application();   // Defined in Application.cpp where Font is complete.

    // Non-copyable, non-moveable — manages unique GPU state.
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    /**
     * @brief Enters the main application render loop.
     */
    void run();

  private:
    /** @brief Loads font and character animation data from asset directory. */
    void initAssets();

    /** @brief Compiles and links SPIR-V/GLSL shader programs. */
    void initShaders();

    /** @brief Constructs 2D quads and particle buffer objects. */
    void initMeshes();

    /** @brief Connects EnTT event dispatcher sinks to application handlers. */
    void setupCallbacks();

    /** @brief Input event handlers. */
    void onKeyEvent(const events::KeyEvent &event);
    void onScrollEvent(const events::ScrollEvent &event);
    void onCursorPosEvent(const events::MouseMovedEvent &event);
    void onMouseButtonEvent(const events::MouseButtonEvent &event);

    /** @brief Per-frame rendering pipeline. */
    void renderFrame(double time);
    void renderBackground(const glm::mat4 &projection);
    void renderCharacter(double time, const glm::mat4 &projection);
    void renderSnow(double time, const glm::mat4 &projection);
    void renderItems(double time, const glm::mat4 &projection);
    void renderUI(double time, const glm::vec2 &winSize);

    // --- Core subsystems (heap-allocated via unique_ptr to allow fwd-decl) ---
    entt::dispatcher                             m_eventDispatcher;
    std::unique_ptr<bgl::window::Window>         m_window;
    std::unique_ptr<bgl::gfx::Camera>            m_camera;
    std::unique_ptr<bgl::gfx::Hud>              m_hud;
    std::unique_ptr<bgl::gfx::TileMap>          m_tileMap;
    std::unique_ptr<bgl::audio::AudioEngine>    m_audioEngine;
    std::unique_ptr<bgl::gfx::Sampler>          m_defaultSampler;

    // --- 3D Scene ---
    std::shared_ptr<bgl::gfx::Scene>            m_scene;
    std::unique_ptr<bgl::gfx::GltfRenderer>     m_gltfRenderer;
    std::unique_ptr<bgl::gfx::Grid>             m_grid;
    std::unique_ptr<bgl::gfx::Camera3D>         m_camera3D;

    // --- Characters ---
    std::vector<std::shared_ptr<Character>>     m_characters;
    size_t                                       m_currentCharacterIndex{0};

    // --- World items ---
    struct WorldItem {
        glm::vec2 pos;
        float     scale;
        float     timeOffset;
    };
    std::vector<WorldItem>                       m_worldItems;
    std::shared_ptr<bgl::gl::Texture2DArray>         m_itemTexture;
    uint32_t                                     m_itemFrameCount{0};

    // --- Font (FreeType) ---
    // unique_ptr so the complete Font type is only required in Application.cpp,
    // not in every TU that includes Application.hpp.
    std::unique_ptr<Font>                        m_font;
    std::unique_ptr<bgl::gl::Texture2DArray>         m_snowTexture;

    // --- Quads (value members — QuadMesh.hpp is lightweight) ---
    QuadMesh m_spriteQuad;
    QuadMesh m_overlayQuad;
    QuadMesh m_bgQuad;

    // --- RAII-wrapped GL handles ---
    bgl::gfx::ProgramHandle m_mainProgram;
    bgl::gfx::ProgramHandle m_textProgram;
    bgl::gfx::ProgramHandle m_bgProgram;
    bgl::gfx::ProgramHandle m_snowProgram;

    bgl::gfx::VAOHandle m_snowVAO;
    bgl::gfx::BufferHandle m_snowVBO;
    bgl::gfx::BufferHandle m_quadVBO;
    bgl::gfx::BufferHandle m_quadIBO;

    // --- Frame timing ---
    double m_lastFpsTime{0.0};
    double m_lastFrameTime{0.0};
    int    m_frameCounter{0};
    int    m_currentFps{60};

    // --- Input state ---
    bool m_leftPressed{false};
    bool m_rightPressed{false};
    bool m_spacePressed{false};

    // --- Physics constants ---
    static constexpr float kCharacterSpeed = 2.0f;
    static constexpr float kGravity        = 15.0f;
    static constexpr float kJumpForce      = 8.0f;
    static constexpr float kGroundY        = 0.0f;
};

} // namespace bgl
