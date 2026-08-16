set(CMAKE_AUTOMOC ON)

add_library(bgl_windowing STATIC
    Window.cpp
    InputHandler.cpp
)

target_compile_features(bgl_windowing PUBLIC cxx_std_23)
target_include_directories(bgl_windowing PUBLIC ${CMAKE_SOURCE_DIR}/src)

find_package(Qt6 COMPONENTS Core Gui OpenGL REQUIRED)

target_link_libraries(bgl_windowing PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    glad::glad
    OpenGL::GL
    EnTT::EnTT
    glm::glm
    spdlog::spdlog
)

target_link_libraries(bgl_windowing PRIVATE bgl_gfx)
