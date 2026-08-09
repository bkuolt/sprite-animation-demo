add_library(bgl_gfx STATIC
    GLContext.cpp
    Graphics.cpp
    QuadMesh.cpp
    TileMap.cpp
    Texture2D.cpp
    Texture2DArray.cpp
    Sampler.cpp
    Camera.cpp
    Hud.cpp
)

target_compile_features(bgl_gfx PUBLIC cxx_std_23)
target_include_directories(bgl_gfx PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(bgl_gfx PUBLIC
    glad::glad
    OpenGL::GL
    glfw
    glm::glm
    KTX::ktx
    spdlog::spdlog
    fmt::fmt
    EnTT::EnTT
    nlohmann_json::nlohmann_json
    bgl_text
)
