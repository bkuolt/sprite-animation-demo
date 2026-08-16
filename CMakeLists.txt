cmake_minimum_required(VERSION 3.20)
project(SpriteAnimationDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Conan dependencies ---
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
find_package(OpenAL REQUIRED)
find_package(fastgltf REQUIRED)
find_package(lua REQUIRED)
find_package(sol2 REQUIRED)

# --- Build options ---
option(BGL_ENABLE_PNG_LOADER  "Enable PNG texture loader"           ON)
option(BGL_ENABLE_JPEG_LOADER "Enable JPEG texture loader"          ON)
option(BGL_ENABLE_GLSL_LOADER "Enable GLSL runtime shader compiler" ON)

# --- Static library modules ---
add_subdirectory(src/gfx/text)    # bgl_text
add_subdirectory(src/io)          # bgl_io
add_subdirectory(src/gl)          # bgl_gl
add_subdirectory(src/gfx)         # bgl_gfx
add_subdirectory(src/audio)       # bgl_audio
add_subdirectory(src/windowing)   # bgl_windowing
add_subdirectory(src/gltf)        # bgl_gltf
add_subdirectory(src/script)      # bgl_script

# --- Application executable ---
add_executable(app
    src/main.cpp
    src/Application.cpp
    src/Character.cpp
)

target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_features(app PRIVATE cxx_std_23)

target_link_libraries(app PRIVATE
    bgl_gl
    bgl_gfx
    bgl_io
    bgl_audio
    bgl_windowing
    bgl_gltf
    bgl_script
    bgl_text
    EnTT::EnTT
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    fmt::fmt
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
    "${CMAKE_SOURCE_DIR}/assets/*.lua"
    "${CMAKE_SOURCE_DIR}/assets/models/*.glb"
)

set(ASSET_DESTINATIONS "")
foreach(ASSET_SOURCE ${ASSET_SOURCES})
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
if(DOXYGEN_FOUND)
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