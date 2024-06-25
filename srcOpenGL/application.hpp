#pragma once

#include "program.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "camera.hpp"
#include "scene.hpp"

namespace glr{

struct ApplicationFPS {
    public:
        bool _DisplayFPS = true;
        uint32_t _NbFramesBetweenDisplay = 10;
        float _LastFrame = 0.f;

    private:
        uint32_t _FrameCounter = 0;
        float _SumOfTimes = 0.f;
        float _MinTime = INFINITY;
        float _MaxTime = 0.f;

        float _AvgFPS = 0.f;
        float _MinFPS = 0.f;
        float _MaxFPS = 0.f;

    public:
        void increment();
        void display();
};


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

struct ApplicationOptions {
    bool _IsWireframeModeOn = false;
    bool _IsBVHDisplayed = false;
    int _DepthDisplayBVH = 0;
};

class Application {
    private:
        ApplicationParameters _Parameters = {};
        ApplicationFPS _FPS = {};
        ApplicationOptions _Options = {};

        GLFWwindow* _Window = nullptr;
        ProgramPtr _RenderingProgram = nullptr;
        ProgramPtr _ComputeProgram = nullptr;
        GLuint _RectangleVao = 0;
        GLuint _ImageTextureId = 0;
        CameraPtr _Camera = nullptr;
        ScenePtr _Scene = nullptr;

    private:
        void initGLFW() const;
        void quitGLFW() const;

        void initWindow();
        void quitWindow() const;

        void initGLAD() const;
        void initViewport() const;
        void initCamera();

        void initShaders();
        void initCallbacks();
        void initScene();

        void initImgui();

        void init();
        void quit() const;

        void mainLoop();
        void processInput() const;

        void drawOneFrame() const;
        void initRectangleVAO();
        void initTexture();

        void render();
        void drawImgui();

    public:
        Application(ApplicationParameters parameters = {});
        ~Application();
        static Application dummyApplication();
        CameraPtr getCamera() const;

    public:
        void run();
        void clearScreen() const;
        void swapBuffers() const;

};

}