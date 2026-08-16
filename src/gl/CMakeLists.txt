find_package(Qt6 COMPONENTS Core Gui OpenGL REQUIRED)

add_library(bgl_gl STATIC GLContext.cpp Sampler.cpp Shader.cpp Texture2D.cpp Texture2DArray.cpp TextureCube.cpp)
target_include_directories(bgl_gl PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(bgl_gl PUBLIC glad::glad KTX::ktx fmt::fmt spdlog::spdlog Qt6::Core Qt6::Gui Qt6::OpenGL)
if(BGL_ENABLE_GLSL_LOADER)
    target_compile_definitions(bgl_gl PUBLIC BGL_ENABLE_GLSL_LOADER)
endif()
