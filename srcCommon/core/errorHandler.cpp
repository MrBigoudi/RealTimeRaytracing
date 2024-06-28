#include "errorHandler.hpp"
#include <GLFW/glfw3.h>

namespace cr{

/**
 * Default error handler
 * @note if level == FATAL, exit the program
 * @note if level == WARNING, print the warning
 * @param msg The error message to display
 * @param level The error level
*/
void ErrorHandler::defaultCase(const std::string& fileName, int lineNumber, const std::string& msg, ErrorLevel level){
    switch(level){
        case FATAL:
            fprintf(stderr, "Error triggered in %s:%d\n\t", fileName.c_str(), lineNumber);
            fprintf(stderr, "%s", msg.c_str());
            fprintf(stderr, "Exiting the program!\n");
            exit(EXIT_FAILURE);
            break;
        case WARNING:
            fprintf(stderr, "Warning triggered in %s:%d\n\t", fileName.c_str(), lineNumber);
            fprintf(stderr, "%s", msg.c_str());
            fprintf(stderr, "Warning, continue the program!\n");
            break;
        default:
            break;
    }
}

/**
 * Handle an error
 * @param error The error to handle
 * @param msg The error message to display
 * @param level The error level
*/
void ErrorHandler::handle(const std::string& fileName, int lineNumber, ErrorCode error, const std::string& msg, ErrorLevel level){
    switch(error){
        case NO_ERROR:
            break;
        default:
            defaultCase(fileName, lineNumber, msg, level);
            break;
    }
}

/**
 * Handle a glfw error
 * @param msg Th error message to display
 * @param level The error level
*/
void ErrorHandler::glfwError(const std::string& fileName, int lineNumber, const std::string& msg, ErrorLevel level){
    glfwTerminate();
    handle(fileName, lineNumber, GLFW_ERROR, msg, level);
}

void ErrorHandler::vulkanError(bool result, const std::string& fileName, int lineNumber, const std::string& msg, ErrorLevel level){
    if(!result){
        handle(fileName, lineNumber, VULKAN_ERROR, msg, level);
    }
}

}