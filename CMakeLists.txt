cmake_minimum_required(VERSION 3.20)
project(SpriteAnimationDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm REQUIRED)  
find_package(Ktx REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

add_executable(app
    src/main.cpp
    src/window.cpp
    src/gfx/shader.cpp
    src/gfx/graphics.cpp
    src/gfx/texture.cpp
    src/loaders/ktx.cpp
    src/loaders/png.cpp
    src/loaders/jpeg.cpp
)

target_link_libraries(app PRIVATE
    glad::glad
    OpenGL::GL
    glfw
    glm::glm
    KTX::ktx
    spdlog::spdlog
    fmt::fmt
    PNG::PNG
    JPEG::JPEG
)

if(MSVC)
    target_compile_options(app PRIVATE /W4 /utf-8)
else()
    target_compile_options(app PRIVATE -Wall -Wextra -Wpedantic)
endif()

# --- Asset Management ---
file(GLOB_RECURSE ASSET_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/assets/*.ktx2"
    "${CMAKE_SOURCE_DIR}/assets/*.spv"
    "${CMAKE_SOURCE_DIR}/assets/*.png"
    "${CMAKE_SOURCE_DIR}/assets/*.jpg"
)

set(ASSET_DESTINATIONS "")
foreach(ASSET_SOURCE ${ASSET_SOURCES})
    get_filename_component(ASSET_FILENAME ${ASSET_SOURCE} NAME)
    set(ASSET_DESTINATION "${CMAKE_BINARY_DIR}/assets/${ASSET_FILENAME}")
    list(APPEND ASSET_DESTINATIONS ${ASSET_DESTINATION})

    add_custom_command(
        OUTPUT ${ASSET_DESTINATION}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASSET_SOURCE}" "${ASSET_DESTINATION}"
        DEPENDS "${ASSET_SOURCE}"
        COMMENT "Copying asset: ${ASSET_FILENAME}"
    )
endforeach()

add_custom_target(copy_assets ALL DEPENDS ${ASSET_DESTINATIONS})
add_dependencies(app copy_assets)