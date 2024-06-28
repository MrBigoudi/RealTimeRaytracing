#pragma once

#include <GLFW/glfw3.h>

#include "camera.hpp"


namespace cr{

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
        static void mouseInput(GLFWwindow* window, CameraPtr camera, double xpos, double ypos);
        static void cameraInput(GLFWwindow* window, CameraPtr camera, float dt);

};

}