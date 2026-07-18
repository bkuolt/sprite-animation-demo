
#include "window.hpp"

#include <spdlog/spdlog.h>
#include <csignal>

static void signal_handler(int signal)  {

    spdlog::info("Received signal: {}", signal);
    // TODO: A global window object or a static instance within the Window class
    // would be needed to call close() here.
}

int main()
{
    spdlog::set_level(spdlog::level::info);

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