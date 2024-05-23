#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#define MAX_NB_TRIANGLES 8 // should match raytracer.glsl

struct TriangleGPU{
    uint32_t _Id;
    uint32_t _MaterialId;
    glm::vec4 _P0;
    glm::vec4 _P1;
    glm::vec4 _P2;
};

class Triangle{
    public:
        TriangleGPU _InternalStruct{};
};