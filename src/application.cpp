#include "application.hpp"
#include "errorHandler.hpp"
#include "input.hpp"
#include "shader.hpp"

Application::Application(ApplicationParameters parameters){
    _Parameters = parameters;
}

void Application::initCamera() {
    float aspectRatio = static_cast<float>(_Parameters._ViewportWidth) / static_cast<float>(_Parameters._ViewportHeight);
    glm::vec3 position = glm::vec3(0.f, 0.f, -5.f);
    _Camera = CameraPtr(new Camera(position, aspectRatio));
}

void Application::initGLFW() const {
    if(glfwInit() != GLFW_TRUE){
        ErrorHandler::glfwError(__FILE__, __LINE__, "Failed to initialize GLFW!\n");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, _Parameters._OpenglVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, _Parameters._OpenglVersionMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, _Parameters._OpenglProfile);
    glfwWindowHint(GLFW_RESIZABLE, _Parameters._WindowIsResizable);
}

void Application::quitGLFW() const {
    glfwTerminate();
}

void Application::initWindow(){
    GLFWmonitor* monitor = nullptr;
    GLFWwindow* share = nullptr;
    _Window = glfwCreateWindow(
                _Parameters._WindowWidth, 
                _Parameters._WindowHeight, 
                _Parameters._WindowTitle.c_str(), 
                monitor, 
                share
            );
    if(_Window == nullptr){
        ErrorHandler::glfwError(__FILE__, __LINE__, "Failed to initialize the window!\n");
    }
    // center window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int screenWidth = mode->width;
    int screenHeight = mode->height;
    int windowXPos = (screenWidth - _Parameters._WindowWidth) / 2;
    int windowYPos = (screenHeight - _Parameters._WindowHeight) / 2;
    glfwSetWindowPos(_Window, windowXPos, windowYPos);

    glfwMakeContextCurrent(_Window);

    // display cursor
    glfwSetInputMode(_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(_Window, this);

    // bypass 60FPS lock
    glfwSwapInterval(0);
}

void Application::quitWindow() const {
    glfwDestroyWindow(_Window);
}

void Application::initGLAD() const{
    if(!gladLoadGL((GLADloadfunc)glfwGetProcAddress)){
        ErrorHandler::handle(
            __FILE__, 
            __LINE__, 
            ErrorCode::GLAD_ERROR,
            "Failed to initialize GLAD!\n"
        );
    }
}

void Application::initViewport() const {
    GLint xOffset = 0;
    GLint yOffset = 0;
    glViewport(
        xOffset, yOffset, 
        _Parameters._ViewportWidth, 
        _Parameters._ViewportHeight
    );
}

void Application::initCallbacks() {
    glfwSetCursorPosCallback(_Window, Input::mouseCallback);
}

void Application::processInput() const {
    if(glfwGetKey(_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(_Window, GLFW_TRUE);
    }
    float dt = glfwGetTime() - _FPS._LastFrame;
    Input::cameraInput(_Window, _Camera, dt);
}

void Application::clearScreen() const {
    glClearColor(
        _Parameters._BackgroundColor.r,
        _Parameters._BackgroundColor.g,
        _Parameters._BackgroundColor.b,
        _Parameters._BackgroundColor.a
    );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Application::initRectangleVAO() {
    // hard coded quad positions
    const GLfloat POSITIONS[] = {
        -1.f, 1.f,
        1.f, 1.f,
        -1.f, -1.f,
        1.f, -1.f
    };

    // hard coded quad texture coords
    const GLfloat TEXTURE_COORDS[] = {
        0.f, 1.f,
        1.f, 1.f,
        0.f, 0.f,
        1.f, 0.f
    };

    GLuint vboPositions, vboTexCoords;

    // Generate VAO
    glGenVertexArrays(1, &_RectangleVao);
    if(_RectangleVao == 0){
        ErrorHandler::handle(
            __FILE__, __LINE__, 
            ErrorCode::OPENGL_ERROR,
            "Failed to generate the rectangle vao!\n"
        );
    }
    glBindVertexArray(_RectangleVao);

    // Generate VBO for positions
    glGenBuffers(1, &vboPositions);
    if(vboPositions == 0){
        ErrorHandler::handle(
            __FILE__, __LINE__, 
            ErrorCode::OPENGL_ERROR,
            "Failed to generate the rectangle's positions vbo!\n"
        );
    }
    glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(POSITIONS), POSITIONS, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Generate VBO for texture coordinates
    glGenBuffers(1, &vboTexCoords);
    if(vboPositions == 0){
        ErrorHandler::handle(
            __FILE__, __LINE__, 
            ErrorCode::OPENGL_ERROR,
            "Failed to generate the rectangle's texture coordinates vbo!\n"
        );
    }
    glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TEXTURE_COORDS), TEXTURE_COORDS, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // Unbind VAO
    glBindVertexArray(0);
}

void Application::initScene() {
    _Scene = ScenePtr(new Scene());

    _Scene->addMaterial({0.2, 0.3, 0.1, 1.});

    // load model
    MeshPtr model = Mesh::load(Mesh::MODELS_DIRECTORY + "teapot.obj");
    // MeshPtr model = Mesh::load(Mesh::MODELS_DIRECTORY + "suzanne.obj");
    model->setMaterial(1);
    _Scene->addMesh(model);

    // // first triangle
    // MeshPtr basicTri = Mesh::primitiveCube();
    // basicTri->setMaterial(1);
    // _Scene->addMesh(basicTri);

    // // random materials
    // for(size_t i=0; i<MAX_NB_MATERIALS; i++){
    //     _Scene->addRandomMaterial();
    // }
    // // random triangles
    // MeshPtr randTri = Mesh::primitiveTriangle();
    // randTri->setMaterial(static_cast<uint32_t>(static_cast<float>(rand())/RAND_MAX*static_cast<float>(MAX_NB_MATERIALS)));
    // randTri->setRotation(
    //     2.f*rand()/RAND_MAX*std::numbers::pi - std::numbers::pi, 
    //     2.f*rand()/RAND_MAX*std::numbers::pi - std::numbers::pi,
    //     2.f*rand()/RAND_MAX*std::numbers::pi - std::numbers::pi
    // );
    // _Scene->addMesh(randTri);
}

CameraPtr Application::getCamera() const{
    if(!_Camera){
        ErrorHandler::handle(
            __FILE__, 
            __LINE__, 
            ErrorCode::NOT_INITIALIZED_ERROR,
            "Can't access an unitilialized camera!\n"
        );
    }
    return _Camera;
}


void Application::drawOneFrame() const {
    // use the compute shader
    assert(_ComputeProgram->isInit());
    _ComputeProgram->use();
    uint32_t nbGroupsX = _Parameters._ViewportWidth / 10.f;
    uint32_t nbGroupsY = _Parameters._ViewportHeight / 10.f;
    uint32_t nbGroupsZ = 1;
    float uniformTimeValue = _FPS._LastFrame;
    _ComputeProgram->setFloat("uTime", uniformTimeValue);
    _ComputeProgram->setBool("uIsWireframeModeOn", true);
    _ComputeProgram->setBool("uIsBVHDisplayed", true);
    _ComputeProgram->setInt("uDepthDisplayBVH", 10);
    // send camera data
    assert(_Camera);
    CameraGPU cameraDataToSend = _Camera->getGpuData();
    _ComputeProgram->setMat4("uCamera._View", cameraDataToSend._View);
    _ComputeProgram->setMat4("uCamera._Proj", cameraDataToSend._Proj);
    _ComputeProgram->setMat4("uCamera._InvView", cameraDataToSend._InvView);
    _ComputeProgram->setMat4("uCamera._InvProj", cameraDataToSend._InvProj);
    _ComputeProgram->setVec4("uCamera._Eye", cameraDataToSend._Eye);
    _ComputeProgram->setFloat("uCamera._PlaneWidth", cameraDataToSend._PlaneWidth);
    _ComputeProgram->setFloat("uCamera._PlaneHeight", cameraDataToSend._PlaneHeight);
    _ComputeProgram->setFloat("uCamera._PlaneNear", cameraDataToSend._PlaneNear);

    glDispatchCompute(nbGroupsX, nbGroupsY, nbGroupsZ);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // use the graphics shadersdt
    assert(_RenderingProgram->isInit());
    _RenderingProgram->use(); 
    assert(_RectangleVao != 0);
    glBindVertexArray(_RectangleVao);
    glActiveTexture(GL_TEXTURE0);
    assert(_ImageTextureId != 0);
    glBindTexture(GL_TEXTURE_2D, _ImageTextureId);
    GLint uniformTextureValue = 0; // 0 for GL_TEXTURE0
    _RenderingProgram->setInt("uRaytracedTexture", uniformTextureValue);
    GLint firstIndex = 0;
    GLsizei numberOfIndices = 4;
    glDrawArrays(GL_TRIANGLE_STRIP, firstIndex, numberOfIndices);
    glBindVertexArray(0);
}

void Application::render() {
    clearScreen();
    drawOneFrame();
    glfwSwapBuffers(_Window);
    _FPS.increment();
    _FPS.display();
}

void Application::initShaders(){
    ShaderPtr vertexShader = ShaderPtr(new Shader(Shader::SHADER_DIRECTORY + "raytracer.vert", VERTEX_SHADER));
    ShaderPtr fragmentShader = ShaderPtr(new Shader(Shader::SHADER_DIRECTORY + "raytracer.frag", FRAGMENT_SHADER));
    _RenderingProgram = ProgramPtr(new Program(vertexShader, fragmentShader));
    ShaderPtr computeShader = ShaderPtr(new Shader(Shader::SHADER_DIRECTORY + "raytracer.glsl", COMPUTE_SHADER));
    _ComputeProgram = ProgramPtr(new Program(computeShader));
}

void Application::init(){
    initGLFW();
    initWindow();
    initGLAD();
    initViewport();
    initShaders();
    initRectangleVAO();
    initTexture();
    initCamera();
    initCallbacks();
    initScene();
    // static scene
    _Scene->sendDataToGpu(_ComputeProgram);
}

void Application::run(){
    init();
    mainLoop();
    quit();
}

void Application::quit() const{
    quitGLFW();
    quitWindow();
}

void Application::mainLoop(){
    while(!glfwWindowShouldClose(_Window)){
        processInput();
        render();
        glfwPollEvents();
    }
}

void Application::initTexture(){
    glGenTextures(1, &_ImageTextureId);
    if(_ImageTextureId == 0){
        ErrorHandler::handle(
            __FILE__, __LINE__, 
            ErrorCode::OPENGL_ERROR,
            "Failed to generate the texture!\n"
        );
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _ImageTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    GLsizei textureWidth = _Parameters._ViewportWidth;
    GLsizei textureHeight = _Parameters._ViewportHeight;
    GLint mipmapLevel = 0;
    GLint border = 0; // must be 0
    glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RGBA32F, textureWidth, textureHeight, border, GL_RGBA, GL_FLOAT, nullptr);

    GLint imageIndex = 0;
    GLboolean isTextureLayered = GL_FALSE;
    GLint textureLayer = 0; // no layer
    GLenum access = GL_READ_ONLY;
    glBindImageTexture(imageIndex, _ImageTextureId, mipmapLevel, isTextureLayered, textureLayer, access, GL_RGBA32F);
}


void ApplicationFPS::increment(){
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - _LastFrame;
    _LastFrame = currentFrame;

    _SumOfTimes += deltaTime;
    _FrameCounter++;

    if(deltaTime > _MaxTime) _MaxTime = deltaTime;
    if(deltaTime < _MinTime) _MinTime = deltaTime;
}

void ApplicationFPS::display(){
    if(_FrameCounter >= _NbFramesBetweenDisplay){
        float avgFPS = 1.f / (_SumOfTimes / static_cast<float>(_FrameCounter));
        float minFPS = 1.f / _MaxTime;
        float maxFPS = 1.f / _MinTime;

        _FrameCounter = 0;
        _SumOfTimes = 0.f;
        _MinTime = INFINITY;
        _MaxTime = 0.f;
        
        if(_DisplayFPS){
            fprintf(stdout,
                "avg FPS: %.2f\nmin FPS: %.2f\nmax FPS: %.2f\n\n", 
                avgFPS, minFPS, maxFPS
            );
        }
    }
}