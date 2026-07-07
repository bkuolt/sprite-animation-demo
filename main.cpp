
#include "Window.hpp"
//#include "ktx.hpp"

#include <iostream>

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

int main()
{
    //spdlog::set_level(spdlog::level::trace);

    try {



    } catch(std::exception& e) {
        //return EXIT_FAILURE;
    }

    try {

        Window window;
        window.run();
    } 
    catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}