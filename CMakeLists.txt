cmake_minimum_required(VERSION 3.15)
project(GLFW_Example CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm REQUIRED)  
find_package(Ktx REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(app src/main.cpp src/window.cpp src/shader.cpp src/ktx.cpp src/graphics.cpp src/texture.cpp)
target_link_libraries(app PRIVATE glad::glad OpenGL::GL glfw glm::glm KTX::ktx spdlog::spdlog fmt::fmt)


# --- Asset Handling (Optimized & Incremental) ---
# This ensures assets are copied to the output directory alongside the executable.
# It's more robust and faster than the previous POST_BUILD step as it only copies changed files.

# 1. Find all asset files that need to be copied at runtime.
file(GLOB KTX_ASSETS "${CMAKE_SOURCE_DIR}/assets/*.ktx2")
file(GLOB SPV_ASSETS "${CMAKE_SOURCE_DIR}/assets/*.spv")

set(ASSET_SOURCES ${KTX_ASSETS} ${SPV_ASSETS})

# 2. For each source asset, define its destination and create a copy command.
set(ASSET_DESTINATIONS "")
foreach(ASSET_SOURCE ${ASSET_SOURCES})
    get_filename_component(ASSET_FILENAME ${ASSET_SOURCE} NAME)
    set(ASSET_DESTINATION "${CMAKE_BINARY_DIR}/assets/${ASSET_FILENAME}")
    list(APPEND ASSET_DESTINATIONS ${ASSET_DESTINATION})

    add_custom_command(
        OUTPUT ${ASSET_DESTINATION}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ASSET_SOURCE}"
                "${ASSET_DESTINATION}"
        DEPENDS "${ASSET_SOURCE}"
        COMMENT "Copying asset if changed: ${ASSET_FILENAME}"
    )
endforeach()

# 3. Create a custom target that groups all asset-copying commands.
# By making it depend on the output files, CMake knows when to re-run the commands.
add_custom_target(copy_assets ALL DEPENDS ${ASSET_DESTINATIONS})

# 4. Make the main application depend on the assets being ready.
add_dependencies(app copy_assets)