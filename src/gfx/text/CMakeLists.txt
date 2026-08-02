add_library(bgl_text STATIC
    Font.cpp
    TextShaper.cpp
    TextRenderer.cpp
    TextureAtlas.cpp
)

target_compile_features(bgl_text PUBLIC cxx_std_23)
target_include_directories(bgl_text PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(bgl_text PUBLIC
    glad::glad
    OpenGL::GL
    glm::glm
    Freetype::Freetype
    harfbuzz::harfbuzz
    Fontconfig::Fontconfig
    stb::stb
    spdlog::spdlog
    fmt::fmt
)
