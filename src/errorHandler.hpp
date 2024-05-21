#pragma once

#include <string>

/**
 * @enum The level of the error
*/
enum ErrorLevel{
    FATAL,
    WARNING,
};

/**
 * @enum The different error codes
*/
enum ErrorCode{
    NO_ERROR, 
    GLAD_ERROR,
    GLFW_ERROR,
    OPENGL_ERROR,
    IO_ERROR,
    NOT_IMPLEMENTED_ERROR,
    USAGE_ERROR,
    BAD_VALUE_ERROR,
};

/**
 * The class to handle errors
*/
class ErrorHandler{
    private:
        /**
         * Private constructor to make the class purely virtual
        */
        ErrorHandler();

    public:
        /**
         * Handle an error
         * @param error The error to handle
         * @param msg The error message to display
         * @param level The error level
        */
        static void handle(const std::string& fileName, int lineNumber, ErrorCode error, const std::string& msg = "", ErrorLevel level = FATAL);

        /**
         * Handle a glfw error
         * @param msg Th error message to display
         * @param level The error level
        */
        static void glfwError(const std::string& fileName, int lineNumber, const std::string& msg, ErrorLevel level = FATAL);

    private:
        /**
         * Default error handler
         * @note if level == FATAL, exit the program
         * @note if level == WARNING, print the warning
         * @param msg The error message to display
         * @param level The error level
        */
        static void defaultCase(const std::string& fileName, int lineNumber, const std::string& msg = "", ErrorLevel level = FATAL);

};