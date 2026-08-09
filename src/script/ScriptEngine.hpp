// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <filesystem>
#include <memory>
#include <string>

// Forward declare sol::state to avoid pulling Lua headers into every TU.
namespace sol { class state; }

namespace bgl::script
{
/**
 * @brief Manages a single Lua interpreter instance via sol2.
 *
 * Exposes a minimal engine API to Lua scripts:
 *  - bgl.log(msg)
 *  - bgl.version() -> string
 */
class ScriptEngine
{
  public:
    ScriptEngine();
    ~ScriptEngine();

    ScriptEngine(const ScriptEngine &) = delete;
    ScriptEngine &operator=(const ScriptEngine &) = delete;

    /**
     * @brief Executes a Lua script file.
     * @throws std::runtime_error on Lua errors.
     */
    void executeFile(const std::filesystem::path &path);

    /**
     * @brief Executes an inline Lua string.
     * @throws std::runtime_error on Lua errors.
     */
    void executeString(const std::string &source);

  private:
    void registerEngineAPI();

    std::unique_ptr<sol::state> m_lua;
};
} // namespace bgl::script
