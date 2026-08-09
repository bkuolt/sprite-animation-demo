add_library(bgl_audio STATIC
    AudioEngine.cpp
    SoundBuffer.cpp
    SoundSource.cpp
)

target_compile_features(bgl_audio PUBLIC cxx_std_23)
target_include_directories(bgl_audio PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(bgl_audio PUBLIC
    OpenAL::OpenAL
    glm::glm
    spdlog::spdlog
)
