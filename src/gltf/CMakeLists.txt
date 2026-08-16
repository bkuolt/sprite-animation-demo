add_library(bgl_gltf STATIC
    Scene.cpp
    Node.cpp
    Mesh.cpp
    GltfLoader.cpp
    GltfRenderer.cpp
    Grid.cpp
    Camera3D.cpp
)

target_compile_features(bgl_gltf PUBLIC cxx_std_23)
target_compile_definitions(bgl_gltf PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_include_directories(bgl_gltf PUBLIC ${CMAKE_SOURCE_DIR}/src)

find_package(fastgltf CONFIG REQUIRED)
find_package(glm REQUIRED)

target_link_libraries(bgl_gltf PUBLIC
    fastgltf::fastgltf
    glm::glm
    glad::glad
    OpenGL::GL
    bgl_io
    spdlog::spdlog
    fmt::fmt
)
