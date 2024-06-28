#include "input.hpp"

namespace cr{

MouseParamters Input::_Mouse = {
    ._Mode = MOUSE_MODE_CAMERA,
    ._NeedsToBeInit = true,
    ._LastX = 0.,
    ._LastY = 0.,
};

void Input::mouseInput(GLFWwindow* window, CameraPtr camera, double xpos, double ypos) {

    if (_Mouse._NeedsToBeInit) {
        _Mouse._LastX = xpos;
        _Mouse._LastY = ypos;
        _Mouse._NeedsToBeInit = false;
    }

    double xoffset = _Mouse._LastX - xpos;
    double yoffset = ypos - _Mouse._LastY; // reversed since y-coordinates range from bottom to top

    _Mouse._LastX = xpos;
    _Mouse._LastY = ypos;

    if(_Mouse._Mode != MOUSE_MODE_CAMERA) return;
    camera->ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}


void Input::cameraInput(GLFWwindow* window, CameraPtr camera, float dt){
    // move camera
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        camera->processKeyboard(FORWARD, dt);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        camera->processKeyboard(BACKWARD, dt);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        camera->processKeyboard(LEFT, dt);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        camera->processKeyboard(RIGHT, dt);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        camera->processKeyboard(UP, dt);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        camera->processKeyboard(DOWN, dt);
    }

    // Switching camera mode
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS){
        if(!_Mouse._MouseModeButtonIsBeingPressed){
            _Mouse._MouseModeButtonIsBeingPressed = true;
            if(_Mouse._Mode == MOUSE_MODE_CAMERA){
                _Mouse._Mode = MOUSE_MODE_WINDOW;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CENTER_CURSOR);
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                _Mouse._Mode = MOUSE_MODE_CAMERA;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE){
        _Mouse._MouseModeButtonIsBeingPressed = false;
    }

    // accelerate camera
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        camera->_Accelerate = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE){
        camera->_Accelerate = false;
    }

}

}