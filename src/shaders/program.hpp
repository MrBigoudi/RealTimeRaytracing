#pragma once

#include <memory>
#include "shader.hpp"
#include <glad/gl.h>
#include <map>

class Program;
using ProgramPtr = std::shared_ptr<Program>;

class Program{
    public:
        void use();
        void setShader(ShaderPtr shader);
        bool isInit() const;

        // TODO: add other variants
        Program(ShaderPtr vertex, ShaderPtr fragment);
        Program(ShaderPtr compute);

        ~Program();

    private:
        GLuint _Id = 0;
        std::map<ShaderType, ShaderPtr> _Shaders;

    private:
        void linkShaders() const;
};