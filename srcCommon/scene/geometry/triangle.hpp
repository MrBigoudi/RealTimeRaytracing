#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace cr{


struct TriangleGPU{
    glm::vec4 _P0;
    glm::vec4 _P1;
    glm::vec4 _P2;
    alignas(16) uint32_t _ModelId;
};

class Triangle{
    private:
        static uint32_t _IdGenerator;

    public:
        static const size_t MAX_NB_TRIANGLES = 2<<15;
        // static const uint32_t MAX_NB_TRIANGLES = 2<<3;

    public:
        TriangleGPU _InternalStruct{};

    public:
        Triangle(const glm::vec4& p0, const glm::vec4& p1, const glm::vec4& p2, uint32_t modelId);
        Triangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, uint32_t modelId);

    public:
        static glm::vec3 getCentroid(const TriangleGPU& triangle);
        static glm::vec3 getCentroid(const TriangleGPU& triangle, const glm::mat4& model);

};

}