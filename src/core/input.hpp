#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>


enum MouseMode{
    MOUSE_MODE_CAMERA,
    MOUSE_MODE_WINDOW,
};

struct MouseParamters {
    MouseMode _Mode;
    bool _NeedsToBeInit = true;
    double _LastX = 0.;
    double _LastY = 0.;
};

class Input{
    private:
        static MouseParamters _Mouse;

    public:
        static void mouseCallback(GLFWwindow* window, double xpos, double ypos);

};