#pragma once

#include "camera.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>


enum MouseMode{
    MOUSE_MODE_CAMERA,
    MOUSE_MODE_WINDOW,
};

struct MouseParamters {
    MouseMode _Mode = MOUSE_MODE_CAMERA;
    bool _MouseModeButtonIsBeingPressed = false;
    bool _NeedsToBeInit = true;
    double _LastX = 0.;
    double _LastY = 0.;
};

class Input{
    private:
        static MouseParamters _Mouse;

    public:
        static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
        static void cameraInput(GLFWwindow* window, CameraPtr camera, float dt);

};