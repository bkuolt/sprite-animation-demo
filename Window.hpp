#ifndef BGL_WINDOW_HPP
#define BGL_WINDOW_HPP

#include "glad/gl.h"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

class Window
{
public:
    Window();
    ~Window();

    void close();
    void run();

protected:
    void registerCallbacks();
    void initializeGLAD();
    void intitializeOpenGL();
    glm::vec2 getScreenSize() const;

    GLFWwindow *_window;
};

#endif // BGL_WINDOW_HPP