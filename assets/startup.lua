-- startup.lua
-- Executed at engine startup. Demonstrates Lua scripting control over 3D Camera & UI Text.

bgl.log("External startup.lua loaded successfully.")
bgl.log("Engine version: " .. bgl.version())

-- Configure 3D Camera via Lua API
if bgl.camera then
    bgl.camera.set_target(0.0, 5.0, 0.0)
    bgl.camera.set_distance(32.0)
    bgl.camera.set_pitch(28.0)
    bgl.camera.set_yaw(45.0)
    
    local px, py, pz = bgl.camera.get_position()
    bgl.log(string.format("Camera initialized at Position: (%.1f, %.1f, %.1f)", px, py, pz))
end

-- Place custom dynamic UI text overlays via Lua API
if bgl.text then
    bgl.text.clear()
    bgl.text.add_colored("⚡ Lua Script Engine v0.8.1 Active", 15.0, 140.0, 255, 215, 0, 255)
    bgl.text.add_colored("🎥 Camera Orbit Distance: 32.0m | Pitch: 28°", 15.0, 175.0, 129, 212, 250, 255)
end
