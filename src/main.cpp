// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "application.hpp"
#include <csignal>
#include <cstdlib>
#include <memory>
#include <spdlog/spdlog.h>

static std::unique_ptr<bgl::Application> g_app;

static void signal_handler(int signal)
{
    spdlog::info("Received signal: {}", signal);
    g_app.reset();
    std::exit(0);
}

int main()
{
    spdlog::set_level(spdlog::level::info);

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try
    {
        g_app = std::make_unique<bgl::Application>();
        g_app->run();
    }
    catch (const std::exception &e)
    {
        spdlog::error("Exception in main: {}", e.what());
        return EXIT_FAILURE;
    }

    g_app.reset();
    return EXIT_SUCCESS;
}