#pragma once

#include "program.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include <glm/glm.hpp>

struct ApplicationParameters {
    uint32_t _OpenglVersionMajor = 4;
    uint32_t _OpenglVersionMinor = 6;
    uint32_t _OpenglProfile = GLFW_OPENGL_CORE_PROFILE;
    bool _WindowIsResizable = GL_FALSE;

    uint32_t _WindowWidth = 1280;
    uint32_t _WindowHeight = 720;
    uint32_t _ViewportWidth = 1280;
    uint32_t _ViewportHeight = 720;
    std::string _WindowTitle = "RayTracing";
    glm::vec4 _BackgroundColor = glm::vec4(0.2f, 0.3f, 0.3f, 1.f);
};

class Application {
    private:
        ApplicationParameters _Parameters = {};
        GLFWwindow* _Window = nullptr;
        ProgramPtr _RenderingProgram = nullptr;
        GLuint _RectangleVao;

    private:
        void initGLFW() const;
        void quitGLFW() const;

        void initWindow();
        void quitWindow() const;

        void initGLAD() const;
        void initViewport() const;

        void initShaders();

        void init();
        void quit() const;

        void mainLoop();
        void processInput() const;

        void clearScreen() const;
        void drawOneFrame() const;
        void initRectangleVAO();
        void render() const;

    public:
        Application(ApplicationParameters parameters = {});

    public:
        void run();

    private:
        // TODO: to remove
        GLuint createSolidColorTexture(float r, float g, float b, float a) const;


};