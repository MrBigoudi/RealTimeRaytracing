#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace glr{

#define MAX_NB_MATERIALS 2<<15


struct MaterialGPU{
    glm::vec4 _Color = {1.f, 1.f, 1.f, 1.f};
};

class Material{
    private:
        static uint32_t _IdGenerator;
        uint32_t _Id = 0;

    public:
        MaterialGPU _InternalStruct{};

    public:
        Material(const glm::vec4& color);
        Material();
};

}