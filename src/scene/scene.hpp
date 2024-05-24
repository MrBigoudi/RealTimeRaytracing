#pragma once

#include <vector>
#include <array>

#include "program.hpp"
#include "triangle.hpp"
#include "material.hpp"

#include <memory>

class Scene;
using ScenePtr = std::shared_ptr<Scene>;

class Scene{
    private:
        uint32_t _TriangleIdGenerator = 0;
        uint32_t _MaterialIdGenerator = 0;
        std::vector<Triangle> _Triangles = {};
        std::vector<Material> _Materials = {};

    public:
        Scene();

    public:
        std::array<TriangleGPU, MAX_NB_TRIANGLES> getTriangleToGPUData() const;
        std::array<MaterialGPU, MAX_NB_MATERIALS> getMaterialToGPUData() const;

        void addTriangle(uint32_t materialId, const glm::vec4& p0, const glm::vec4& p1, const glm::vec4& p2);
        void addMaterial(const glm::vec4& color);

        void sendDataToGpu(ProgramPtr program);

    private:
        void createUBO();
};