
#include "Window.hpp"

#include <spdlog/spdlog.h>
#include <csignal>

static void signal_handler(int signal)  {
    spdlog::info("Received signal: {}", signal);
    // TODO: window.close();
}

int main()
{
    spdlog::set_level(spdlog::level::trace);

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {
        Window window;
        window.run();
    } 
    catch(std::exception& e) {
        spdlog::error("{}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}