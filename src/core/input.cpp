#include "input.hpp"

#include "application.hpp"

MouseParamters Input::_Mouse = {
    ._Mode = MOUSE_MODE_CAMERA,
    ._NeedsToBeInit = true,
    ._LastX = 0.,
    ._LastY = 0.,
};

void Input::mouseCallback(GLFWwindow* window, double xpos, double ypos) {

    if(_Mouse._Mode != MOUSE_MODE_CAMERA) return;

    if (_Mouse._NeedsToBeInit) {
        _Mouse._LastX = xpos;
        _Mouse._LastY = ypos;
        _Mouse._NeedsToBeInit = false;
    }

    double xoffset = _Mouse._LastX - xpos;
    double yoffset = ypos - _Mouse._LastY; // reversed since y-coordinates range from bottom to top

    _Mouse._LastX = xpos;
    _Mouse._LastY = ypos;

    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    assert(app);
    app->getCamera()->ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}