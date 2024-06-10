#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#define MAX_NB_TRIANGLES 2<<5

struct TriangleGPU{
    glm::vec4 _P0;
    glm::vec4 _P1;
    glm::vec4 _P2;
    alignas(16) uint32_t _Id;
    uint32_t _MaterialId;
    uint32_t _ModelId;
};

class Triangle{
    private:
        static uint32_t _IdGenerator;

    public:
        TriangleGPU _InternalStruct{};

    public:
        Triangle(const glm::vec4& p0, const glm::vec4& p1, const glm::vec4& p2, uint32_t modelId, uint32_t materialId = 0);
        Triangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, uint32_t modelId, uint32_t materialId = 0);
};