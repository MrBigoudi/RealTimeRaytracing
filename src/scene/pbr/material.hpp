#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#define MAX_NB_MATERIALS 2<<15


struct MaterialGPU{
    glm::vec4 _Color;
    alignas(16) uint32_t _Id;
};

class Material{
    public:
        MaterialGPU _InternalStruct{};

    public:
        Material(uint32_t id, const glm::vec4& color);
        Material(uint32_t id);
};