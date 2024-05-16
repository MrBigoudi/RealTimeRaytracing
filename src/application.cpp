#include "application.hpp"
#include "errorHandler.hpp"
#include "shader.hpp"

Application::Application(ApplicationParameters parameters){
    _Parameters = parameters;
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
    glfwMakeContextCurrent(_Window);
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

void Application::processInput() const {
    if(glfwGetKey(_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(_Window, GLFW_TRUE);
    }
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
        -1.f, -1.f,
        1.f, -1.f,
        -1.f, 1.f,
        1.f, 1.f
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
    glBindVertexArray(_RectangleVao);

    // Generate VBO for positions
    glGenBuffers(1, &vboPositions);
    glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(POSITIONS), POSITIONS, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Generate VBO for texture coordinates
    glGenBuffers(1, &vboTexCoords);
    glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TEXTURE_COORDS), TEXTURE_COORDS, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // Unbind VAO
    glBindVertexArray(0);
}

void Application::drawOneFrame() const {
    // use the graphics shaders
    _RenderingProgram->use();

    // create the texture
    float r = 1.f;
    float g = 0.f;
    float b = 0.f;
    float a = 1.f;
    GLuint colorTexture = createSolidColorTexture(r,g,b,a); // Red color
    // matches the binding location in the shader
    GLuint bindingLocation = 0;
    glBindTextureUnit(bindingLocation, colorTexture);    
    
    glBindVertexArray(_RectangleVao);
    GLint firstIndex = 0;
    GLsizei numberOfIndices = 4;
    glDrawArrays(GL_TRIANGLE_STRIP, firstIndex, numberOfIndices);
    glBindVertexArray(0);
}

void Application::render() const{
    clearScreen();
    drawOneFrame();
    glfwSwapBuffers(_Window);
}

void Application::initShaders(){
    ShaderPtr vertexShader = ShaderPtr(new Shader(Shader::SHADER_DIRECTORY + "raytracer.vert", VERTEX_SHADER));
    ShaderPtr fragmentShader = ShaderPtr(new Shader(Shader::SHADER_DIRECTORY + "raytracer.frag", FRAGMENT_SHADER));
    _RenderingProgram = ProgramPtr(new Program(vertexShader, fragmentShader));
}

void Application::init(){
    initGLFW();
    initWindow();
    initGLAD();
    initViewport();
    initShaders();
    initRectangleVAO();
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


// TODO: to remove
// Function to create a simple color texture
GLuint Application::createSolidColorTexture(float r, float g, float b, float a) const {
    // Define the color data
    size_t bytesPerPixel = 4;
    size_t dataSize = _Parameters._ViewportWidth * _Parameters._ViewportWidth * bytesPerPixel;
    unsigned char* textureData = new unsigned char[dataSize];

    unsigned char red = static_cast<unsigned char>(r * 255);
    unsigned char green = static_cast<unsigned char>(g * 255);
    unsigned char blue = static_cast<unsigned char>(b * 255);
    unsigned char alpha = static_cast<unsigned char>(a * 255);

    // Fill the texture data with the solid color
    for (size_t i = 0; i < dataSize; i += 4) {
        textureData[i] = red;
        textureData[i + 1] = green;
        textureData[i + 2] = blue;
        textureData[i + 3] = alpha;
    }

    GLuint texture;
    GLsizei numberOfTextures = 1;
    glCreateTextures(GL_TEXTURE_2D, numberOfTextures, &texture);

    GLsizei numberOfTextureLevels = 1; // for mipmapping
    glTextureStorage2D(
        texture, 
        numberOfTextureLevels, 
        GL_RGBA8, 
        _Parameters._ViewportWidth, 
        _Parameters._ViewportHeight
    );

    GLint levelOfDetailNumber = 0; // for mipmapping
    GLint xOffset = 0;
    GLint yOffset = 0;
    glTextureSubImage2D(
        texture, 
        levelOfDetailNumber, 
        xOffset, yOffset, 
        _Parameters._ViewportWidth, 
        _Parameters._ViewportHeight, 
        GL_RGBA, 
        GL_UNSIGNED_BYTE, 
        textureData
    );

    return texture;
}