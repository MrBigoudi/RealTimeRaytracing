#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "shader.hpp"
#include <glad/gl.h>
#include <map>

class Program;
using ProgramPtr = std::shared_ptr<Program>;

class Program{
    public:
        void use() const;
        void setShader(ShaderPtr shader);
        bool isInit() const;

        // TODO: add other variants
        Program(ShaderPtr vertex, ShaderPtr fragment);
        Program(ShaderPtr compute);

        ~Program();

    public:
        GLuint getLocation(const std::string & name) const;
        void setBool(const std::string & name, bool value) const;
        void setFloat(const std::string & name, float value) const;
        void setInt(const std::string & name, int value) const;
        void setUInt(const std::string & name, unsigned int value) const;
        void setVec2(const std::string & name, const glm::vec2 & value) const;
        void setVec3(const std::string & name, const glm::vec3 & value) const;
        void setVec4(const std::string & name, const glm::vec4 & value) const;
        void setMat4(const std::string & name, const glm::mat4 & value) const;
        GLuint getId() const;

    private:
        GLuint _Id = 0;
        std::map<ShaderType, ShaderPtr> _Shaders;

    private:
        void linkShaders() const;
};