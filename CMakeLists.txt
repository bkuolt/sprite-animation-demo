cmake_minimum_required(VERSION 3.15)
project(GLFW_Example CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm REQUIRED)  
find_package(Ktx REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(app main.cpp window.cpp shader.cpp ktx.cpp)
target_link_libraries(app PRIVATE glad::glad OpenGL::GL glfw glm::glm KTX::ktx spdlog::spdlog fmt::fmt)