add_library(bgl_gl STATIC GLContext.cpp Sampler.cpp Shader.cpp Texture2D.cpp Texture2DArray.cpp)
target_include_directories(bgl_gl PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(bgl_gl PUBLIC glad::glad KTX::ktx fmt::fmt spdlog::spdlog)
