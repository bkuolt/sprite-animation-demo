add_library(bgl_io STATIC
    KtxLoader.cpp
    ShaderLoader.cpp
)

if(BGL_ENABLE_PNG_LOADER)
    target_sources(bgl_io PRIVATE PngLoader.cpp)
endif()

if(BGL_ENABLE_JPEG_LOADER)
    target_sources(bgl_io PRIVATE JpegLoader.cpp)
endif()

target_compile_features(bgl_io PUBLIC cxx_std_23)
target_include_directories(bgl_io PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(bgl_io PUBLIC
    glad::glad
    OpenGL::GL
    KTX::ktx
    spdlog::spdlog
    fmt::fmt
)

if(BGL_ENABLE_PNG_LOADER)
    target_link_libraries(bgl_io PUBLIC PNG::PNG pngpp::pngpp)
    target_compile_definitions(bgl_io PUBLIC BGL_ENABLE_PNG_LOADER)
endif()

if(BGL_ENABLE_JPEG_LOADER)
    target_link_libraries(bgl_io PUBLIC JPEG::JPEG)
    target_compile_definitions(bgl_io PUBLIC BGL_ENABLE_JPEG_LOADER)
endif()

if(BGL_ENABLE_GLSL_LOADER)
    target_compile_definitions(bgl_io PUBLIC BGL_ENABLE_GLSL_LOADER)
endif()
