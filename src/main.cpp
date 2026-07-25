// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "window.hpp"
#include "gfx/graphics.hpp"
#include "gfx/shader.hpp"
#include "loaders/ktx.hpp"

#include <spdlog/spdlog.h>
#include <csignal>
#include <cmath>
#include <vector>
#include <array>
#include <filesystem>
#include <memory>

extern int currentAnimation;

static std::unique_ptr<Window> g_window;

static void signal_handler(int signal)
{
    spdlog::info("Received signal: {}", signal);
    if (g_window)
    {
        g_window->close();
    }
}

int main()
{
    spdlog::set_level(spdlog::level::info);

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try
    {
        g_window = std::make_unique<Window>();

        const auto binaryPath = std::filesystem::read_symlink("/proc/self/exe");
        const auto basePath = binaryPath.parent_path();

        // 1. Load textures using polymorphic ITextureLoader
        const std::array fileNames{
            basePath / "assets" / "idle.ktx2",
            basePath / "assets" / "walk.ktx2",
            basePath / "assets" / "jump.ktx2",
            basePath / "assets" / "run.ktx2",
            basePath / "assets" / "slide.ktx2",
            basePath / "assets" / "dead.ktx2"};

        std::vector<std::unique_ptr<bgl::ITextureLoader>> loaders;
        loaders.reserve(fileNames.size());
        for (const auto &file : fileNames)
        {
            loaders.push_back(std::make_unique<bgl::ktx::Loader>(file, KTX_TTF_BC3_RGBA));
        }

        std::vector<GLuint> textureIDs(loaders.size());
        for (size_t i = 0; i < loaders.size(); ++i)
        {
            textureIDs[i] = loaders[i]->upload();
        }

        // 2. Load SPIR-V shaders
        const auto vsSpv = bgl::LoadSPIRVShaderFromFile(basePath / "assets" / "main.vert.spv");
        const auto fsSpv = bgl::LoadSPIRVShaderFromFile(basePath / "assets" / "main.frag.spv");
        const GLuint program = bgl::CreateShaderProgramFromSPIRV(vsSpv, fsSpv);

        // 3. Create mesh
        const bgl::QuadMesh quadMesh = bgl::create2DQuad();

        // 4. Register render callback in main.cpp
        g_window->setRenderCallback([&](double time)
        {
            const auto currentTexture = static_cast<size_t>(currentAnimation) % textureIDs.size();

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            const auto c = (static_cast<int>(time) % 10) / 10.0f;
            glClearColor(c * 0.2f, c * 0.1f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            GLint layers = 0;
            glGetTextureLevelParameteriv(textureIDs[currentTexture], 0, GL_TEXTURE_DEPTH, &layers);

            const auto num_frames = layers > 0 ? layers : 1;
            constexpr double target_fps = 24.0;

            const double totalFrames = time * target_fps;
            const int currentFrame = static_cast<int>(std::floor(totalFrames)) % num_frames;
            const float tweenFactor = static_cast<float>(totalFrames - std::floor(totalFrames));

            bgl::renderQuad(quadMesh, textureIDs[currentTexture], program, currentFrame, tweenFactor);
        });

        g_window->run();
    }
    catch (const std::exception &e)
    {
        spdlog::error("{}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}