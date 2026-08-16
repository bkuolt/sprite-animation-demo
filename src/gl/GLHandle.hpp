// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <glad/gl.h>
#include <utility>

namespace bgl::gl
{
/**
 * @brief Type-safe RAII wrapper for a single OpenGL handle.
 *
 * Template parameter `Deleter` is a stateless callable that takes (GLuint)
 * and calls the appropriate glDelete* function.
 *
 * Usage:
 *   auto deleteProgram  = [](GLuint h) { glDeleteProgram(h); };
 *   GLHandle<decltype(deleteProgram)> program;
 *   program.reset(glCreateProgram());
 *
 * Or use the provided type aliases:
 *   ProgramHandle program;
 *   program.reset(glCreateProgram());
 */
template <auto Deleter>
class GLHandle
{
  public:
    constexpr GLHandle() noexcept = default;
    constexpr explicit GLHandle(GLuint handle) noexcept : m_handle(handle) {}

    ~GLHandle() { reset(); }

    GLHandle(const GLHandle &) = delete;
    GLHandle &operator=(const GLHandle &) = delete;

    constexpr GLHandle(GLHandle &&other) noexcept : m_handle(other.m_handle)
    {
        other.m_handle = 0;
    }

    constexpr GLHandle &operator=(GLHandle &&other) noexcept
    {
        if (this != &other)
        {
            reset();
            m_handle = other.m_handle;
            other.m_handle = 0;
        }
        return *this;
    }

    /// Deletes the current handle (if valid) and takes ownership of a new one.
    void reset(GLuint handle = 0) noexcept
    {
        if (m_handle != 0) Deleter(m_handle);
        m_handle = handle;
    }

    /// Releases ownership without deleting. Caller takes responsibility.
    [[nodiscard]] GLuint release() noexcept
    {
        return std::exchange(m_handle, 0);
    }

    [[nodiscard]] constexpr GLuint get()     const noexcept { return m_handle; }
    [[nodiscard]] constexpr         operator GLuint() const noexcept { return m_handle; }
    [[nodiscard]] constexpr explicit operator bool()  const noexcept { return m_handle != 0; }

  private:
    GLuint m_handle{0};
};

// --- Deleters (as inline constexpr lambdas — C++20) ---

inline constexpr auto kDeleteProgram = [](GLuint h) noexcept { glDeleteProgram(h); };
inline constexpr auto kDeleteVAO     = [](GLuint h) noexcept { glDeleteVertexArrays(1, &h); };
inline constexpr auto kDeleteBuffer  = [](GLuint h) noexcept { glDeleteBuffers(1, &h); };
inline constexpr auto kDeleteTexture = [](GLuint h) noexcept { glDeleteTextures(1, &h); };

// --- Convenient type aliases ---

using ProgramHandle = GLHandle<kDeleteProgram>;
using VAOHandle     = GLHandle<kDeleteVAO>;
using BufferHandle  = GLHandle<kDeleteBuffer>;
using TextureHandle = GLHandle<kDeleteTexture>;

} // namespace bgl::gl
