add_library(bgl_script STATIC ScriptEngine.cpp)

target_compile_features(bgl_script PUBLIC cxx_std_23)
target_include_directories(bgl_script PUBLIC ${CMAKE_SOURCE_DIR}/src)

find_package(lua REQUIRED)
find_package(sol2 REQUIRED)

target_link_libraries(bgl_script PUBLIC
    lua::lua
    sol2::sol2
    spdlog::spdlog
    bgl_gltf
    bgl_gfx
)

target_compile_definitions(bgl_script PUBLIC SOL_ALL_SAFETIES_ON=1)
