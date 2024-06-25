#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application.hpp"

namespace vkr{

void Application::initGLFW(){
    if(glfwInit() != GLFW_TRUE){
        fprintf(
            stderr,
            "Failed to initialize GLFW!\n"
        );
        exit(EXIT_FAILURE);
    }
}

void Application::destroyGLFW(){
    glfwTerminate();
}

void Application::initWindow(){
    GLFWmonitor* monitor = nullptr;
    GLFWwindow* share = nullptr;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _Window = glfwCreateWindow(
                _Parameters._WindowWidth, 
                _Parameters._WindowHeight, 
                _Parameters._WindowTitle.c_str(), 
                monitor, 
                share
            );
    if(_Window == nullptr){
        fprintf(
            stderr,
            "Failed to initialize the window!\n"
        );
        exit(EXIT_FAILURE);
    }
    // center window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int screenWidth = mode->width;
    int screenHeight = mode->height;
    int windowXPos = (screenWidth - _Parameters._WindowWidth) / 2;
    int windowYPos = (screenHeight - _Parameters._WindowHeight) / 2;
    glfwSetWindowPos(_Window, windowXPos, windowYPos);

    // display cursor
    glfwSetInputMode(_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(_Window, this);

    // bypass 60FPS lock
    glfwSwapInterval(0);
}

void Application::destroyWindow(){
    glfwDestroyWindow(_Window);
}

}