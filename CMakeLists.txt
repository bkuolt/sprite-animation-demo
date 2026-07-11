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

add_executable(app src/main.cpp src/window.cpp src/shader.cpp src/ktx.cpp src/graphics.cpp src/texture.cpp src/shader.cpp)
target_link_libraries(app PRIVATE glad::glad OpenGL::GL glfw glm::glm KTX::ktx spdlog::spdlog fmt::fmt)


# copy asssets dir to buil dir and also the shaders (main.n,fs, main.vs) to the build dir
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:app>/assets
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/src/main.vs $<TARGET_FILE_DIR:app>/
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/src/main.fs $<TARGET_FILE_DIR:app>/
)