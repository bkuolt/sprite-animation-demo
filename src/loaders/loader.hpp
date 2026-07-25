// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include <glad/gl.h>

namespace bgl
{
    /**
     * @brief Interface for polymorphic texture loaders.
     */
    class ITextureLoader
    {
    public:
        virtual ~ITextureLoader() = default;

        /**
         * @brief Uploads loaded texture data to the GPU.
         * @return OpenGL handle for the created GL_TEXTURE_2D_ARRAY texture.
         */
        [[nodiscard]] virtual GLuint upload() = 0;
    };
} // namespace bgl
