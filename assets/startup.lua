-- startup.lua
-- This script is executed at engine startup before the main loop begins.
-- Use it for project-level configuration, feature flags, or debug hooks.

bgl.log("External startup.lua loaded successfully.")
bgl.log("Engine version: " .. bgl.version())
