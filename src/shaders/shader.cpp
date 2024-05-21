#include "shader.hpp"

#include <fstream>
#include <sstream>

#include "errorHandler.hpp"

const std::string Shader::SHADER_DIRECTORY = std::string(PROJECT_SOURCE_DIR) + "/src/shaders/shaderCodes/";

Shader::~Shader(){
    glDeleteShader(_Id);
}

GLuint Shader::getId() const{
    return _Id;
};

ShaderType Shader::getType() const {
    return _Type;
}

const std::string Shader::readShaderFile() const{
    std::string shaderCode;
    std::ifstream shaderFile;

    try {
        shaderFile.open(_FilePath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        shaderCode = shaderStream.str();
    }
    catch(std::ifstream::failure &e){
        ErrorHandler::handle(
            __FILE__, 
            __LINE__, 
            IO_ERROR,
            "Failed to read the file: " + _FilePath + "%s!\n"
        );
    }
    return shaderCode;
}


void Shader::compileShader(const std::string& shaderCode){
    int success;
    char infoLog[512];

    switch(_Type){
        case VERTEX_SHADER:
            _Id = glCreateShader(GL_VERTEX_SHADER);
            break;
        case FRAGMENT_SHADER:
            _Id = glCreateShader(GL_FRAGMENT_SHADER);
            break;
        case COMPUTE_SHADER:
            _Id = glCreateShader(GL_COMPUTE_SHADER);
            break;
        default:
            ErrorHandler::handle(
                __FILE__, 
                __LINE__, 
                ErrorCode::NOT_IMPLEMENTED_ERROR,
                "Could not compile " + _FilePath + "; this type of shader is not yet implemented!\n"
            );
    }

    const char* codeCStr = shaderCode.c_str();
    GLsizei numberOfShaders = 1;
    const GLint* stringLengths = nullptr; // nullptr if only one shader
    glShaderSource(_Id, numberOfShaders, &codeCStr, stringLengths);
    glCompileShader(_Id);
    glGetShaderiv(_Id, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(_Id, 512, NULL, infoLog);
        ErrorHandler::handle(
            __FILE__,
            __LINE__,
            ErrorCode::OPENGL_ERROR,
            "Failed to compile the shader `" + _FilePath + "': " + infoLog + "!\n"
        );
    };

}

Shader::Shader(const std::string& path, ShaderType type){
    _FilePath = path;
    _Type = type;

    const std::string code = readShaderFile();
    compileShader(code);
}