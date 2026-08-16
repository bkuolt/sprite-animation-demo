add_library(bgl_windowing STATIC
    Window.cpp
    InputHandler.cpp
)

target_compile_features(bgl_windowing PUBLIC cxx_std_23)
target_include_directories(bgl_windowing PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(bgl_windowing PUBLIC
    glfw
    glad::glad
    OpenGL::GL
    EnTT::EnTT
    glm::glm
    spdlog::spdlog
)

# GLContext (InitializeGLAD, InitializeOpenGL) lives in bgl_gfx.
# We use an INTERFACE-only dependency here to avoid a circular dep:
# bgl_gfx -> bgl_text -> (nothing), bgl_windowing -> bgl_gfx (GLContext only).
# If this ever becomes circular, extract GLContext into its own micro-library.
target_link_libraries(bgl_windowing PRIVATE bgl_gfx)
