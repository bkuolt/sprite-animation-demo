// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "ScriptEngine.hpp"
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

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
        return "BGL Engine 0.1 (C++23 / OpenGL 4.6)";
    });
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
