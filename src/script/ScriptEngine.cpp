// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "ScriptEngine.hpp"
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

#include "gfx/Hud.hpp"
#include "gltf/Camera3D.hpp"
#include <tuple>

namespace bgl::script
{
ScriptEngine::ScriptEngine()
    : m_lua(std::make_unique<sol::state>())
{
    m_lua->open_libraries(
        sol::lib::base,
        sol::lib::math,
        sol::lib::string,
        sol::lib::table,
        sol::lib::io
    );
    registerEngineAPI();
}

ScriptEngine::~ScriptEngine() = default;

void ScriptEngine::registerEngineAPI()
{
    auto bgl = m_lua->create_named_table("bgl");

    bgl.set_function("log", [](const std::string &msg) {
        spdlog::info("[Lua] {}", msg);
    });

    bgl.set_function("version", []() -> std::string {
        return "BGL Engine 0.8.1 (C++23 / OpenGL 4.6 DSA / QML & Lua)";
    });
}

void ScriptEngine::bindCamera(bgl::gfx::Camera3D *camera)
{
    if (!camera) return;

    auto bgl = (*m_lua)["bgl"].get_or_create<sol::table>();
    auto cameraTable = bgl.create_named("camera");

    cameraTable.set_function("set_target", [camera](float x, float y, float z) {
        camera->setTarget(glm::vec3(x, y, z));
    });

    cameraTable.set_function("set_distance", [camera](float dist) {
        camera->setDistance(dist);
    });

    cameraTable.set_function("set_pitch", [camera](float pitch) {
        camera->setPitch(pitch);
    });

    cameraTable.set_function("set_yaw", [camera](float yaw) {
        camera->setYaw(yaw);
    });

    cameraTable.set_function("get_position", [camera]() -> std::tuple<float, float, float> {
        auto pos = camera->getPosition();
        return {pos.x, pos.y, pos.z};
    });

    spdlog::info("[ScriptEngine] Bound 3D Camera API to Lua (bgl.camera)");
}

void ScriptEngine::bindHud(bgl::gfx::Hud *hud)
{
    if (!hud) return;

    auto bgl = (*m_lua)["bgl"].get_or_create<sol::table>();
    auto textTable = bgl.create_named("text");

    textTable.set_function("add", [hud](const std::string &msg, float x, float y) {
        hud->addText(msg, x, y);
    });

    textTable.set_function("add_colored", [hud](const std::string &msg, float x, float y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        hud->addText(msg, x, y, glm::u8vec4(r, g, b, a));
    });

    textTable.set_function("clear", [hud]() {
        hud->clearCustomTexts();
    });

    spdlog::info("[ScriptEngine] Bound Text / HUD Overlay API to Lua (bgl.text)");
}

void ScriptEngine::executeFile(const std::filesystem::path &path)
{
    auto result = m_lua->script_file(path.string(), [](lua_State *, sol::protected_function_result pfr) {
        return pfr;
    });

    if (!result.valid())
    {
        sol::error err = result;
        throw std::runtime_error(std::string("Lua error in '") + path.string() + "': " + err.what());
    }
}

void ScriptEngine::executeString(const std::string &source)
{
    auto result = m_lua->script(source, [](lua_State *, sol::protected_function_result pfr) {
        return pfr;
    });

    if (!result.valid())
    {
        sol::error err = result;
        throw std::runtime_error(std::string("Lua error: ") + err.what());
    }
}
} // namespace bgl::script
