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
find_package(freetype REQUIRED)
find_package(harfbuzz REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(pngpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(EnTT REQUIRED)
find_package(stb REQUIRED)

option(BGL_ENABLE_PNG_LOADER "Enable PNG texture loader" ON)
option(BGL_ENABLE_JPEG_LOADER "Enable JPEG texture loader" OFF)
option(BGL_ENABLE_GLSL_LOADER "Enable GLSL runtime shader compiler" ON)

set(APP_SOURCES
    src/main.cpp
    src/Application.cpp
    src/windowing/Window.cpp
    src/windowing/InputHandler.cpp
    src/Character.cpp
    src/gfx/Graphics.cpp
    src/gfx/Texture2D.cpp
    src/gfx/Texture2DArray.cpp
    src/gfx/Sampler.cpp
    src/gfx/Camera.cpp
    src/gfx/Hud.cpp
)

# Subdirectories for static libraries
add_subdirectory(src/gfx/text)
add_subdirectory(src/io)

add_executable(app ${APP_SOURCES})

target_link_libraries(app PRIVATE
    glad::glad
    OpenGL::GL
    glfw
    glm::glm
    spdlog::spdlog
    fmt::fmt
    nlohmann_json::nlohmann_json
    EnTT::EnTT
    bgl_text
    bgl_io
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
    "${CMAKE_SOURCE_DIR}/assets/*.vs"
    "${CMAKE_SOURCE_DIR}/assets/*.fs"
    "${CMAKE_SOURCE_DIR}/assets/*.png"
    "${CMAKE_SOURCE_DIR}/assets/*.jpg"
    "${CMAKE_SOURCE_DIR}/assets/*.ttf"
    "${CMAKE_SOURCE_DIR}/assets/*.otf"
    "${CMAKE_SOURCE_DIR}/assets/*.json"
)

set(ASSET_DESTINATIONS "")
foreach(ASSET_SOURCE ${ASSET_SOURCES})
    get_filename_component(ASSET_REL_PATH ${ASSET_SOURCE} NAME)
    # The GLOB_RECURSE gives absolute paths. We just extract the filename and put it in build/assets/
    # Actually wait, since we restructured to assets/textures/ktx, etc., it's better to preserve the relative path.
    file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}/assets" "${ASSET_SOURCE}")
    set(ASSET_DESTINATION "${CMAKE_BINARY_DIR}/assets/${REL_PATH}")
    get_filename_component(DEST_DIR ${ASSET_DESTINATION} DIRECTORY)
    list(APPEND ASSET_DESTINATIONS ${ASSET_DESTINATION})

    add_custom_command(
        OUTPUT "${ASSET_DESTINATION}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASSET_SOURCE}" "${ASSET_DESTINATION}"
        DEPENDS "${ASSET_SOURCE}"
        COMMENT "Copying asset: ${REL_PATH}"
        VERBATIM
    )
endforeach()

add_custom_target(copy_assets ALL DEPENDS ${ASSET_DESTINATIONS})
add_dependencies(app copy_assets)

# --- Documentation ---
find_package(Doxygen)
if (DOXYGEN_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      doxygen-awesome-css
      GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
      GIT_TAG v2.3.3
    )
    FetchContent_MakeAvailable(doxygen-awesome-css)

    set(DOXYGEN_AWESOME_CSS_DIR ${doxygen-awesome-css_SOURCE_DIR})

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()