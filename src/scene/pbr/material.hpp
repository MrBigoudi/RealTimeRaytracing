#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#define MAX_NB_MATERIALS 8 // should match raytracer.glsl


struct MaterialGPU{
    uint32_t _Id;
    glm::vec4 _Color;
};

class Material{
    public:
        MaterialGPU _InternalStruct{};
};