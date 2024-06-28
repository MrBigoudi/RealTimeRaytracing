#include "program.hpp"

#include <algorithm>
#include <cassert>
#include <glm/ext.hpp>
#include <vector>
#include "errorHandler.hpp"

namespace glr{

void Program::use() const {
    glUseProgram(_Id);
}

Program::~Program(){
    glDeleteProgram(_Id);
    glUseProgram(0);
}

void Program::setShader(ShaderPtr shader){
    switch(shader->getType()){
        case COMPUTE_SHADER:
            cr::ErrorHandler::handle(__FILE__, __LINE__, cr::ErrorCode::USAGE_ERROR, "Can't set a compute shader!\n");
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

    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if(!success){
        GLint maxLength = 0;
        glGetProgramiv(_Id, GL_INFO_LOG_LENGTH, &maxLength);
        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(_Id, maxLength, &maxLength, &infoLog[0]);
        // The program is useless now. So delete it.
        glDeleteProgram(_Id);
        
        std::string errorMessage(infoLog.begin(), infoLog.end());
        cr::ErrorHandler::handle(
            __FILE__,
            __LINE__,
            cr::ErrorCode::OPENGL_ERROR,
            "Failed to link the shaders: " + errorMessage + "!\n"
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
        cr::ErrorHandler::handle(
            __FILE__, 
            __LINE__, 
            cr::ErrorCode::OPENGL_ERROR,
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
        cr::ErrorHandler::handle(
            __FILE__, 
            __LINE__, 
            cr::ErrorCode::OPENGL_ERROR,
            "Failed to create the program!\n"
        );
    }
    linkShaders();
}

bool Program::isInit() const {
    return _Id != 0;
}

GLuint Program::getLocation(const std::string & name) const { 
    use(); 
    return glGetUniformLocation(_Id, name.c_str()); 
}

void Program::setBool(const std::string & name, bool value) const { 
    use(); 
    glUniform1i(getLocation(name.c_str()), value ? 1 : 0); 
}

void Program::setFloat(const std::string & name, float value) const { 
    use(); 
    glUniform1f(getLocation(name.c_str()), value); 
}

void Program::setInt(const std::string & name, int value) const { 
    use(); 
    glUniform1i(getLocation(name.c_str()), value); 
}

void Program::setUInt(const std::string & name, unsigned int value) const { 
    use(); 
    glUniform1ui(getLocation(name.c_str()), int(value)); 
}

void Program::setVec2(const std::string & name, const glm::vec2 & value) const { 
    use(); 
    glUniform2fv(getLocation(name.c_str()), 1, glm::value_ptr(value)); 
}

void Program::setVec3(const std::string & name, const glm::vec3 & value) const { 
    use(); 
    glUniform3fv(getLocation(name.c_str()), 1, glm::value_ptr(value)); 
}

void Program::setVec4(const std::string & name, const glm::vec4 & value) const { 
    use(); 
    glUniform4fv(getLocation(name.c_str()), 1, glm::value_ptr(value)); 
}

void Program::setMat4(const std::string & name, const glm::mat4 & value) const { 
    use(); 
    glUniformMatrix4fv(getLocation(name.c_str()), 1, GL_FALSE, glm::value_ptr(value)); 
}

GLuint Program::getId() const{
    return _Id;
}

}
