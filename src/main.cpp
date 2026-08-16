// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Application.hpp"
#include "script/ScriptEngine.hpp"
#include <QGuiApplication>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>

// Global application pointer — used only by signal handlers.
static std::unique_ptr<bgl::Application> g_app;

static void signal_handler(int sig)
{
    spdlog::info("Received signal {}, shutting down.", sig);
    // Destroy application state safely from signal context.
    // NOTE: Only async-signal-safe operations are truly valid here;
    //       this is acceptable for a graceful dev/desktop exit.
    g_app.reset();
    std::_Exit(0);
}

int main(int argc, char *argv[])
{
    spdlog::set_level(spdlog::level::info);

    QGuiApplication app(argc, argv);

    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    try
    {
        // Run the startup Lua script before initializing the engine.
        // This allows project-level configuration and early scripting hooks.
        bgl::script::ScriptEngine scripts;
        scripts.executeString(R"lua(
            bgl.log("Startup script running.")
            bgl.log("Engine: " .. bgl.version())
        )lua");

        // Optionally load an external startup script if present next to the binary.
        const auto binaryPath = std::filesystem::read_symlink("/proc/self/exe");
        const auto startupScript = binaryPath.parent_path() / "assets" / "startup.lua";
        if (std::filesystem::exists(startupScript))
        {
            scripts.executeFile(startupScript);
        }

        g_app = std::make_unique<bgl::Application>();
        g_app->run();
    }
    catch (const std::exception &e)
    {
        spdlog::error("Fatal exception: {}", e.what());
        return EXIT_FAILURE;
    }

    g_app.reset();
    return EXIT_SUCCESS;
}