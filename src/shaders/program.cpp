#include "program.hpp"

#include <algorithm>
#include <cassert>
#include "errorHandler.hpp"

void Program::use(){
    glUseProgram(_Id);
}

Program::~Program(){
    glDeleteProgram(_Id);
    glUseProgram(0);
}

void Program::setShader(ShaderPtr shader){
    switch(shader->getType()){
        case COMPUTE_SHADER:
            ErrorHandler::handle(__FILE__, __LINE__, ErrorCode::USAGE_ERROR, "Can't set a compute shader!\n");
            break;
        default:
            _Shaders[shader->getType()] = shader;
    }
    linkShaders();
}

void Program::linkShaders() const {
    const GLuint id = _Id;
    std::for_each(_Shaders.begin(), _Shaders.end(), 
    [id](std::pair<ShaderType, ShaderPtr> pair){
        glAttachShader(id, pair.second->getId());
    });

    int success;
    char infoLog[512];

    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        ErrorHandler::handle(
            __FILE__,
            __LINE__,
            ErrorCode::OPENGL_ERROR,
            "Failed to link the shaders!\n"
        );
    };
}

Program::Program(ShaderPtr vertex, ShaderPtr fragment){
    assert(vertex->getType() == VERTEX_SHADER);
    assert(fragment->getType() == FRAGMENT_SHADER);
    _Shaders[VERTEX_SHADER] = vertex;
    _Shaders[FRAGMENT_SHADER] = fragment;
    _Id = glCreateProgram();
    if(_Id == 0){
        ErrorHandler::handle(
            __FILE__, 
            __LINE__, 
            ErrorCode::OPENGL_ERROR,
            "Failed to create the program!\n"
        );
    }
    linkShaders();
}

Program::Program(ShaderPtr compute){
    assert(compute->getType() == COMPUTE_SHADER);
    _Shaders[COMPUTE_SHADER] = compute;
    _Id = glCreateProgram();
    if(_Id == 0){
        ErrorHandler::handle(
            __FILE__, 
            __LINE__, 
            ErrorCode::OPENGL_ERROR,
            "Failed to create the program!\n"
        );
    }
    linkShaders();
}

bool Program::isInit() const {
    return _Id != 0;
}