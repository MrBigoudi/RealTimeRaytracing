#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#define MAX_NB_TRIANGLES 2<<10

struct TriangleGPU{
    glm::vec4 _P0;
    glm::vec4 _P1;
    glm::vec4 _P2;
    alignas(16) uint32_t _Id;
    uint32_t _MaterialId;
};

class Triangle{
    public:
        TriangleGPU _InternalStruct{};

    public:
        Triangle(uint32_t id, uint32_t materialId, const glm::vec4& p0, const glm::vec4& p1, const glm::vec4& p2);
        Triangle(uint32_t id, uint32_t materialId);
};