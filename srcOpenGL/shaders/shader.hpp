#pragma once

#include <memory>
#include <string>
#include <glad/gl.h>

namespace glr{

class Shader;
using ShaderPtr = std::shared_ptr<Shader>;

enum ShaderType{
    VERTEX_SHADER, 
    FRAGMENT_SHADER, 
    GEOMETRY_SHADER,
    TESSELLATION_CONTROL_SHADER,
    TESSELLATION_EVALUATION_SHADER,
    COMPUTE_SHADER,
};

class Shader{

    private:
        std::string _FilePath;
        ShaderType _Type;
        GLuint _Id = 0;

    public:
        static const std::string SHADER_DIRECTORY;

    public:
        Shader(const std::string& path, ShaderType type);
        ~Shader();

        GLuint getId() const;
        ShaderType getType() const;

    private:
        const std::string readShaderFile() const;
        void compileShader(const std::string& shaderCode);

};

}